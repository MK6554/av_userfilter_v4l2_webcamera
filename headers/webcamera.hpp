#ifndef WEBCAMERA
#define WEBCAMERA
#include "paraqueue.hpp"
#include <atomic>
#include <chrono>
#include <thread>

#include "V4l2Capture.h"

class WebCamera {
public:
  void start_acquisition();
  bool grab_image(avl::Image &image);
  void close_acquisition();
  WebCamera(int cameraIndex, int width, int height, int framerate, long exposure_ms);
  ~WebCamera();

  int camera_index() const { return m_camera_index; }
  int max_framerate() const { return m_max_framerate; }
  int width() const { return m_width; }
  int height() const { return m_height; }
  bool can_grab() const;
  bool is_running() const { return m_running; }
  double get_fps() const { return m_queue.get_fps(); }
  std::chrono::milliseconds exposure() const { return m_exposure; }

  void set_max_framerate(int new_max);
  void set_exposure(long time_ms);
  void set_exposure(std::chrono::milliseconds millis);

private:
  std::chrono::milliseconds m_exposure;
  int m_width, m_height, m_max_framerate;
  int m_camera_index;

  V4l2Capture* video_capture;
  char* buffer;
  timeval tv;

  std::thread m_cap_thread;
  std::atomic<bool> m_running;
  ParaQueue m_queue;
  void captureLoop();
};
void list_supported_framerates(std::string const &devicePath);

#endif /* WEBCAMERA */
