#ifndef UF_QUEUE
#define UF_QUEUE
#include <AVL_Lite.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <opencv2/core.hpp>
#include <queue>
using CLOCK = std::chrono::steady_clock;
using UNIT = std::chrono::duration<double, std::milli>;

class ParaQueue {
public:
  void push(const cv::Mat &image);
  bool pop(avl::Image &image);
  bool has_enqueued() const;
  bool is_empty() const;
  ParaQueue();
  ParaQueue(double max_freq);
  double get_fps() const;
  double max_allowed_frequency() const { return m_max_allowed_frequency_hz; }
  void set_max_frequency_hz(double max_freq);

private:
  double m_max_allowed_frequency_hz;
  int m_max_queue_size;
  mutable std::mutex m_mutex;
  mutable std::mutex m_period_mutex;
  std::queue<cv::Mat> m_queue;
  std::queue<std::chrono::time_point<CLOCK>> m_time_queue;
  UNIT m_period;
  void update_times();
};
#endif
