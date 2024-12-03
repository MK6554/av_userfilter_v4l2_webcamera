#ifndef UF_QUEUE
#define UF_QUEUE
#include <AVL_Lite.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <queue>

using CLOCK = std::chrono::steady_clock;

class ParaQueue {
public:
  void push(cv::Mat const &image);
  bool pop(avl::Image &image);
  bool has_enqueued() const;
  bool is_empty() const;
  ParaQueue(int q_size);

private:
  int max_queue_size;
  mutable std::mutex m_mutex;
  std::queue<avl::Image> m_queue;
  std::queue<std::chrono::time_point<CLOCK>> time_queue;
  std::atomic<std::chrono::duration<std::chrono::milliseconds>> period;
};
#endif