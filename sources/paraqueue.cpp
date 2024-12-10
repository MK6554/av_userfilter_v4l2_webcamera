#include "paraqueue.hpp"
#include "debug_log.hpp"
#include <AVLConverters/AVL_OpenCV.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>

void ParaQueue::update_times() {
  auto now = CLOCK::now();
  m_time_queue.push(now);
  while (m_time_queue.size() > 100) {
    m_time_queue.pop();
  }
  auto newest = m_time_queue.back();
  auto oldest = m_time_queue.front();
  auto diff = std::chrono::duration_cast<UNIT>(newest - oldest);
  auto mean = diff / m_time_queue.size();
  auto p = mean;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  m_period = p;
}

bool check_if_too_fast(std::queue<std::chrono::time_point<CLOCK>> const &times,
                       double max_freq) {
  using namespace std::chrono_literals;
  if (max_freq < 0) {
    return false;
  }
  auto now = CLOCK::now();
  auto diff = now - times.back();
  auto is_too_fast_sonic_factor = diff < (1s / max_freq);
  if (is_too_fast_sonic_factor) {
    std::cout << std::chrono::duration_cast<
                     std::chrono::duration<double, std::milli>>(diff)
                     .count()
              << " vs "
              << std::chrono::duration_cast<
                     std::chrono::duration<double, std::milli>>(1s / max_freq)
                     .count()
              << std::endl;
  }
  return is_too_fast_sonic_factor;
}

void ParaQueue::push(const cv::Mat &image) {
  if (check_if_too_fast(m_time_queue, m_max_allowed_frequency_hz)) {
    return;
  }
  // auto tt =
  // std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  // std::cout <<"| "<< tt<<" |"<<std::endl;
  std::lock_guard<std::mutex> lock(m_mutex);
  update_times();
  m_queue.push(std::move(image));
  while (m_queue.size() > m_max_queue_size) {
    m_queue.pop();
  }
}
bool ParaQueue::pop(avl::Image &image) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_queue.empty()) {
    return false;
  }
  auto bgr = m_queue.front();
  cv::Mat rgb;
  cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
  avl::CVMatToAvlImage(rgb, image);
  m_queue.pop();
  return true;
}

bool ParaQueue::has_enqueued() const { return !m_queue.empty(); }
bool ParaQueue::is_empty() const { return m_queue.empty(); }

ParaQueue::ParaQueue() : m_max_allowed_frequency_hz(-1) {}
ParaQueue::ParaQueue(double max_freq) : m_max_allowed_frequency_hz(max_freq) {}

double ParaQueue::get_fps() const {
  using namespace std::chrono_literals;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  return std::chrono::duration_cast<UNIT>(1s) / m_period;
  return m_period.count();
}

void ParaQueue::set_max_frequency_hz(double max_freq) {
  m_max_allowed_frequency_hz = max_freq;
}
