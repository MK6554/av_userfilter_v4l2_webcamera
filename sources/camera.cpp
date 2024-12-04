#include "camera.hpp"
#include "paraqueue.hpp"
#include <AVL_Lite.h>
#include <UserFilter.h>
#include <UserFilterLog.h>
#include <chrono>
#include <debug_log.hpp>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/videodev2.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

void sleep(int time_ms)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(time_ms));
}
void WebCamera::start_acqisition()
{
  running = true;
  capThread = std::thread(&WebCamera::captureLoop, this);
}

bool WebCamera::grab_image(avl::Image &image)
{
  log("Has enqueued: " + std::to_string(queue.has_enqueued()));
  log("Is running: " + std::to_string(running));
  if (queue.has_enqueued())
  {
    return queue.pop(image);
  }
  return false;
}

bool WebCamera::grab_image_sync()
{
  cv::VideoCapture cap(0, cv::CAP_V4L);
  cv::Mat mat;
  while (true)
  {
    cap.retrieve(mat);
    std::cout << mat.size << std::endl;
  }
}

void WebCamera::close_acqisition()
{
  running = false;
  if (capThread.joinable())
  {
    capThread.join();
  }
  if (cap.isOpened())
  {
    cap.release();
  }
}

WebCamera::WebCamera(int cameraIndex, int width, int height, int framerate)
    : queue(8), cameraIndex(cameraIndex), width(width), height(height), framerate(framerate),m_received_frames(0) {}

WebCamera::~WebCamera() { close_acqisition(); }

void WebCamera::captureLoop()
{
  cap.open(cameraIndex, cv::CAP_V4L);
  cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
  cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);
  sleep(50);
  cap.set(cv::CAP_PROP_FPS, framerate);
  sleep(50);
  if (!cap.isOpened())
  {
    running = false;
    return;
    err("Could not connect to the camera.");
  }
  cv::Mat frame;
  while (running)
  {
    if (!cap.grab())
    {
      continue;
    }
    cap >> frame;
    m_received_frames++;
    queue.push(frame);
  }
}

void list_supported_framerates(const std::string &devicePath)
{
  int fd = open(devicePath.c_str(), O_RDWR);
  if (fd == -1)
  {
    std::cerr << "Error: Cannot open device " << devicePath << std::endl;
    return;
  }

  struct v4l2_frmivalenum frameInterval;
  struct v4l2_fmtdesc formatDesc;

  formatDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for (formatDesc.index = 0; ioctl(fd, VIDIOC_ENUM_FMT, &formatDesc) == 0;
       formatDesc.index++)
  {
    std::cout << "Pixel Format: " << formatDesc.description << std::endl;

    struct v4l2_frmsizeenum frameSize;
    frameSize.pixel_format = formatDesc.pixelformat;
    for (frameSize.index = 0;
         ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frameSize) == 0;
         frameSize.index++)
    {
      if (frameSize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        std::cout << "  Resolution: " << frameSize.discrete.width << "x"
                  << frameSize.discrete.height << std::endl;

        frameInterval.pixel_format = formatDesc.pixelformat;
        frameInterval.width = frameSize.discrete.width;
        frameInterval.height = frameSize.discrete.height;
        for (frameInterval.index = 0;
             ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameInterval) == 0;
             frameInterval.index++)
        {
          if (frameInterval.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
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