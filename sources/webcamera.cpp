#include "webcamera.hpp"
#include "paraqueue.hpp"
#include <AVL_Lite.h>
#include <UserFilter.h>
#include <UserFilterLog.h>
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/videodev2.h>

#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#define MAX_DIM 65535

void sleep(int time_ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
}

void wait(TIMEPOINT from, double max_freq_hz) {
  if (max_freq_hz < 0)
    return;
  
  auto min_period = std::chrono::seconds(1) / max_freq_hz;
  std::this_thread::sleep_until(min_period + from);
}

void WebCamera::update_camera_external_timer() { 
  // Time per frame in usec
  double one_dev_by_fps = 1. / this->m_max_framerate;
  one_dev_by_fps = std::floor(one_dev_by_fps * 10000.0) / 10000.0;
  int calculated_usec = one_dev_by_fps * 10000000;
  
  this->tv.tv_usec = calculated_usec;
  this->tv.tv_sec  = 0;
}

void WebCamera::start_acquisition() {
  m_running = true;

  auto width = m_width < 0 ? MAX_DIM : m_width;
  auto height = m_height < 0 ? MAX_DIM : m_height;

  //path to camera device
  char in_devname[50];// = "buffer for path /dev/video%d"
  sprintf(in_devname, "/dev/video%d", m_camera_index);

  V4L2DeviceParameters param(in_devname, V4L2_PIX_FMT_MJPG, width, height, m_max_framerate, V4l2IoType::IOTYPE_MMAP, O_RDWR | O_NONBLOCK, V4l2ExposureMode::Auto);
  this->video_capture = V4l2Capture::create(param);

  if(!this->video_capture) {
    this->m_running = false;

    //In case when we canot init captue we shuld crash
    std::cout << "Camera Not initialized, or missing" << std::endl;
    throw new atl::IoError("Camera Not initialized, or missing");

    return;
  }

  // Allocating buffer
  this->buffer = new char[this->video_capture->getBufferSize()];

  // update capture timer;
  update_camera_external_timer();

  m_cap_thread = std::thread(&WebCamera::captureLoop, this);
}

bool WebCamera::grab_image(avl::Image &image) {
  if (m_queue.has_enqueued())
    return m_queue.pop(image);
  return false;
}

void WebCamera::close_acquisition() {
  m_running = false;
  if (m_cap_thread.joinable())
    m_cap_thread.join();
  
  delete this->video_capture; // releasing camera in destructor
  delete this->buffer; //releasing image buffer
}

bool WebCamera::can_grab() const {
  return m_queue.has_enqueued(); 
}

void WebCamera::set_max_framerate(int new_max) {
  if (this->m_max_framerate != new_max) {

    this->m_max_framerate = new_max;

    // Stoping capture thread
    m_running = false;
    if (m_cap_thread.joinable())
      m_cap_thread.join();

    sleep(10);

    //Stoping video_capture setting new fps and restarting
    this->video_capture->stop();
    sleep(10);
    this->video_capture->setFps(this->m_max_framerate);
    this->m_queue.set_max_frequency_hz(this->m_max_framerate);

    sleep(10);
    this->video_capture->start();

    // update capture timer;
    update_camera_external_timer();

    sleep(5);

    // Starting capture thread
    m_running = true;
    m_cap_thread = std::thread(&WebCamera::captureLoop, this);
  }
}

void WebCamera::set_exposure(long time_ms) {  
  //std::cout << this->m_exposure << std::endl;
  if (this->m_exposure != time_ms)
  {
    if (time_ms == -1)
      this->video_capture->setExposureMode(V4l2ExposureMode::Auto);
    else{
      if (this->m_exposure == -1) // this if is unnecessary
        this->video_capture->setExposureMode(V4l2ExposureMode::Manual);
      this->video_capture->setExposureTime(time_ms);
    }

    this->m_exposure = time_ms;
  }
}

WebCamera::WebCamera(int m_cameraIndex, int width, int height, int framerate,
                     long exposure_ms)
    : m_queue(framerate), m_camera_index(m_cameraIndex), m_width(width),
      m_height(height), m_max_framerate(framerate), m_running(false),
      m_exposure(exposure_ms) {}

WebCamera::~WebCamera() { close_acquisition(); }

void WebCamera::captureLoop() 
{
  while (m_running) {
    if (video_capture->isReadable(&tv)) {
      int rsize = video_capture->read(this->buffer, video_capture->getBufferSize());

      if (rsize == -1)
      {
        this->m_running = false;
        if (m_cap_thread.joinable())
          m_cap_thread.join();

        std::cout << "Can't read data from Camera" << std::endl;
        throw new atl::IoError("Can't read data from Camera");

        return;
      }

      avl::Image outImage;

      avl::LoadImageFromArray(reinterpret_cast<atl::byte*> (this->buffer), rsize, false, outImage);
      //outImage.MakeDataOwn(); // <---- Scary memory menagment boooooo
      m_queue.push(outImage);
    }
    
    //unnecessary since video_capture->isReadable
    //wait(m_queue.last_frame_time(), m_max_framerate);
  }
}

void list_supported_framerates(const std::string &devicePath) {
  int fd = open(devicePath.c_str(), O_RDWR);
  if (fd == -1) {
    std::cerr << "Error: Cannot open device " << devicePath << std::endl;
    return;
  }

  struct v4l2_frmivalenum frameInterval;
  struct v4l2_fmtdesc formatDesc;

  formatDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for (formatDesc.index = 0; ioctl(fd, VIDIOC_ENUM_FMT, &formatDesc) == 0;
       formatDesc.index++) {
    std::cout << "Pixel Format: " << formatDesc.description << std::endl;

    struct v4l2_frmsizeenum frameSize;
    frameSize.pixel_format = formatDesc.pixelformat;
    for (frameSize.index = 0;
         ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize) == 0;
         frameSize.index++) {
      if (frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
        std::cout << "  Resolution: " << frameSize.discrete.width << "x"
                  << frameSize.discrete.height << std::endl;

        frameInterval.pixel_format = formatDesc.pixelformat;
        frameInterval.width = frameSize.discrete.width;
        frameInterval.height = frameSize.discrete.height;
        for (frameInterval.index = 0;
             ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameInterval) == 0;
             frameInterval.index++) {
          if (frameInterval.type == V4L2_FRMIVAL_TYPE_DISCRETE) {
            std::cout << "    Frame Rate: "
                      << (float)frameInterval.discrete.denominator /
                             frameInterval.discrete.numerator
                      << " FPS" << std::endl;
          }
        }
      }
    }
  }

  close(fd);
}
