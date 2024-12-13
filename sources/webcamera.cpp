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
#include <opencv2/opencv.hpp>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#define MAX_DIM 65535

const int MJPEG = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');

void sleep(int time_ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
}
void wait(TIMEPOINT from, double max_freq_hz) {
  if (max_freq_hz < 0) {
    return;
  }
  auto min_period = std::chrono::seconds(1) / max_freq_hz;
  std::this_thread::sleep_until(min_period + from);
}
void WebCamera::start_acquisition() {
  m_running = true;
  m_cap_thread = std::thread(&WebCamera::captureLoop, this);
}

bool WebCamera::grab_image(avl::Image &image) {
  if (m_queue.has_enqueued()) {
    return m_queue.pop(image);
  }
  return false;
}

void WebCamera::close_acquisition() {
  m_running = false;
  if (m_cap_thread.joinable()) {
    m_cap_thread.join();
  }
  if (m_capture.isOpened()) {
    m_capture.release();
  }
}

double WebCamera::get_property(int property_id) const {
  return m_capture.get(property_id);
}

bool WebCamera::set_property(int property_id, double value) {
  return m_capture.set(property_id, value);
}

bool WebCamera::can_grab() const { return m_queue.has_enqueued(); }

void WebCamera::set_max_framerate(int new_max) {
  m_max_framerate = new_max;
  m_queue.set_max_frequency_hz(new_max);
  set_property(cv::CAP_PROP_FPS, new_max);
}

void WebCamera::set_exposure(long time_ms) {
  set_exposure(std::chrono::milliseconds(time_ms));
}

void WebCamera::set_exposure(std::chrono::milliseconds millis) {
  m_exposure = millis;
  auto auto_exp = millis.count() < 0 ? 3 : 1;
  set_property(cv::CAP_PROP_EXPOSURE, m_exposure.count());
  set_property(cv::CAP_PROP_AUTO_EXPOSURE, auto_exp);
}

WebCamera::WebCamera(int m_cameraIndex, int width, int height, int framerate,
                     long exposure_ms)
    : m_queue(framerate), m_camera_index(m_cameraIndex), m_width(width),
      m_height(height), m_max_framerate(framerate), m_running(false),
      m_exposure(exposure_ms) {}

WebCamera::~WebCamera() { close_acquisition(); }
void WebCamera::captureLoop() {
  // sudo apt install uvcdynctrl
  // uvcdynctrl -c -v
  m_capture.open(m_camera_index, cv::CAP_V4L);
  m_capture.set(cv::CAP_PROP_FOURCC, MJPEG);
  auto width = m_width < 0 ? MAX_DIM : m_width;
  auto height = m_height < 0 ? MAX_DIM : m_height;
  m_capture.set(cv::CAP_PROP_FRAME_WIDTH, width);
  m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, height);
  m_capture.set(cv::CAP_PROP_EXPOSURE, m_exposure.count());
  m_capture.set(cv::CAP_PROP_AUTO_EXPOSURE,
                1); // 1 means manual mode (at least for some cameras)

  // cap.set(cv::CAP_PROP_EXPOSURE, height);
  sleep(50);
  m_capture.set(cv::CAP_PROP_FPS, m_max_framerate);
  if (!m_capture.isOpened()) {
    m_running = false;
    return;
  }
  cv::Mat frame;
  while (m_running) {
    if (!m_capture.grab()) {
      continue;
    }
    // cv::Mat _img;
    m_capture.retrieve(frame);
    m_queue.push(frame);
    wait(m_queue.last_frame_time(), m_max_framerate);
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
