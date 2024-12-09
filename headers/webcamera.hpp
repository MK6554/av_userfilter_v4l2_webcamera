#ifndef UF_WEB_CAMERA
#define UF_WEB_CAMERA
#include "paraqueue.hpp"
#include <atomic>
#include <opencv2/opencv.hpp>
#include <thread>
class WebCamera {
public:
  void start_acqisition();
  int camera_index() const { return m_cameraIndex; }
  bool grab_image(avl::Image &image);
  void close_acquisition();
  double get_property(int property_id) const;
  bool set_property(int property_id, double value);
  bool isRunning() const { return running; }
  double get_fps() const { return queue.get_fps(); }
  bool can_grab() const;
  size_t received_frames() const { return m_received_frames.load(); }
  WebCamera(int cameraIndex, int width, int height, int framerate);
  ~WebCamera();

private:
  std::atomic<size_t> m_received_frames;
  int width, height, framerate;
  int m_cameraIndex;
  cv::VideoCapture cap;
  std::thread capThread;
  std::atomic<bool> running;
  ParaQueue queue;
  void captureLoop();
};
void list_supported_framerates(std::string const &devicePath);

#endif