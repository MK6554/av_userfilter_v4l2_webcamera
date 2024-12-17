#ifndef UF_QUEUE
#define UF_QUEUE
#include <AVL_Lite.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>
using CLOCK = std::chrono::steady_clock;
using UNIT = std::chrono::duration<double, std::milli>;
using TIMEPOINT =  std::chrono::time_point<CLOCK>;
class ParaQueue {
public:
  void push(const avl::Image &image);
  bool pop(avl::Image &image);
  bool has_enqueued() const;
  bool is_empty() const;
  ParaQueue();
  ParaQueue(double max_freq);
  double get_fps() const;
  double max_allowed_frequency() const { return m_max_allowed_frequency_hz; }
  void set_max_frequency_hz(double max_freq);
  TIMEPOINT last_frame_time();

private:
  double m_max_allowed_frequency_hz;
  int m_max_queue_size;
  mutable std::mutex m_mutex;
  mutable std::mutex m_period_mutex;
  std::queue<avl::Image> m_queue;
  std::queue<TIMEPOINT> m_time_queue;
  UNIT m_period;
  void update_times();
};
#endif
