#include "webcamera.hpp"
#include "paraqueue.hpp"
#include <AVL_Lite.h>
#include <UserFilter.h>
#include <UserFilterLog.h>
#include <chrono>
#include <debug_log.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#ifdef PLATFORM_UNIX
#include <fcntl.h>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <sys/ioctl.h>
#include <unistd.h>
void sleep(int time_ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
}
void WebCamera::start_acqisition() {
  log("Started acquisition of camera " + std::to_string(m_cameraIndex));
  running = true;
  capThread = std::thread(&WebCamera::captureLoop, this);
}

bool WebCamera::grab_image(avl::Image &image) {
  if (queue.has_enqueued()) {
    return queue.pop(image);
  }
  return false;
}



void WebCamera::close_acquisition() {
  running = false;
  if (capThread.joinable()) {
    capThread.join();
  }
  if (cap.isOpened()) {
    cap.release();
  }
  log("Closed acquisition of camera " + std::to_string(m_cameraIndex));
}

double WebCamera::get_property(int property_id) const {
  return cap.get(property_id);
}

bool WebCamera::can_grab() const { return queue.enqueued_frames() > 0; }

WebCamera::WebCamera(int m_cameraIndex, int width, int height, int framerate)
    : queue(8), m_cameraIndex(m_cameraIndex), width(width), height(height),
      framerate(framerate), m_received_frames(0), running(false) {}

WebCamera::WebCamera(int m_cameraIndex, int width, int height, int framerate,
                     size_t queue_size)
    : queue(queue_size), m_cameraIndex(m_cameraIndex), width(width),
      height(height), framerate(framerate), m_received_frames(0),
      running(false) {}

WebCamera::~WebCamera() { close_acquisition(); }
void WebCamera::captureLoop() {
  cap.open(m_cameraIndex, cv::CAP_V4L);
  cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
  cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
  // cap.set(cv::CAP_PROP_EXPOSURE, height);
  sleep(50);
  cap.set(cv::CAP_PROP_FPS, framerate);
  sleep(50);
  log("Size: " + std::to_string((int)cap.get(cv::CAP_PROP_FRAME_WIDTH)) +
      " x " + std::to_string((int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)));
  log("Exposure: " + std::to_string((int)cap.get(cv::CAP_PROP_EXPOSURE)));
  if (!cap.isOpened()) {
    running = false;
    return;
    err("Could not connect to the camera.");
  }
  cv::Mat frame;
  while (running) {
    if (!cap.grab()) {
      continue;
    }
    // cv::Mat _img;
    cap.retrieve(frame);
    m_received_frames++;
    queue.push(std::move(frame));
    // sleep(15);
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
#else
void WebCamera::start_acqisition() {}
bool WebCamera::grab_image(avl::Image &image) {
  avl::TestImage(avl::TestImageId::Baboon, image, atl::NIL);
  return true;
}
void WebCamera::close_acquisition() {}
double WebCamera::get_property(int property_id) const { return -1.0; };
WebCamera::WebCamera(int m_cameraIndex, int width, int height, int framerate)
    : queue(8), m_cameraIndex(m_cameraIndex), width(width), height(height),
      framerate(framerate), m_received_frames(0) {}
bool WebCamera::can_grab() const { return true; }
WebCamera::WebCamera(int m_cameraIndex, int width, int height, int framerate,
                     size_t queue_size)
    : queue(queue_size), m_cameraIndex(m_cameraIndex), width(width),
      height(height), framerate(framerate), m_received_frames(0) {}
WebCamera::~WebCamera() {}
#endif
