#ifndef UF_QUEUE
#define UF_QUEUE
#include <AVL_Lite.h>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include "opencv.hpp"

using CLOCK = std::chrono::steady_clock;
using UNIT = std::chrono::duration<double,std::milli>;

class ParaQueue {
public:
  void push(cv::Mat const &image);
  bool pop(avl::Image &image);
  bool has_enqueued() const;
  bool is_empty() const;
  ParaQueue(int q_size);
  double get_fps() const;
  size_t enqueued_frames()const{return m_queue.size();}

private:
  int max_queue_size;
  mutable std::mutex m_mutex;
  mutable std::mutex m_period_mutex;
  std::queue<avl::Image> m_queue;
  std::queue<std::chrono::time_point<CLOCK>> time_queue;
  UNIT period;
  void update_times();
};
#endif
