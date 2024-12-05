#include "paraqueue.hpp"
#include "debug_log.hpp"
#include <AVLConverters/AVL_OpenCV.h>
#include <iostream>
#include <mutex>

void ParaQueue::update_times() {
  auto now = CLOCK::now();
  time_queue.push(now);
  while(time_queue.size()>100){
    time_queue.pop();
  }
  auto newest = time_queue.back();
  auto oldest = time_queue.front();
  auto diff = std::chrono::duration_cast<UNIT>(newest - oldest);
  auto mean = diff / time_queue.size();
  auto  p  = mean;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  period = p;
}

void ParaQueue::push(cv::Mat const &image) {
  std::lock_guard<std::mutex> lock(m_mutex);
  avl::Image avlimg;
  // avl::CVMatToAvlImage_Switched(image, avlimg);
  m_queue.push(avlimg);
  update_times();
  while (m_queue.size() > max_queue_size) {
    m_queue.pop();
  }
}

bool ParaQueue::pop(avl::Image &image) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_queue.empty()) {
    return false;
  }
  image = m_queue.front();
  m_queue.pop();
  return true;
}

bool ParaQueue::has_enqueued() const { return !m_queue.empty(); }
bool ParaQueue::is_empty() const { return m_queue.empty(); }

ParaQueue::ParaQueue(int q_size) : max_queue_size(q_size) {}

double ParaQueue::get_fps() const { 
  using namespace std::chrono_literals;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  return std::chrono::duration_cast<UNIT>(1s) / period;
  return period.count();
}
