#include "paraqueue.hpp"
#include "debug_log.hpp"
#include <AVLConverters/AVL_OpenCV.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
void ParaQueue::update_times() {
  auto now = CLOCK::now();
  time_queue.push(now);
  while (time_queue.size() > 100) {
    time_queue.pop();
  }
  auto newest = time_queue.back();
  auto oldest = time_queue.front();
  auto diff = std::chrono::duration_cast<UNIT>(newest - oldest);
  auto mean = diff / time_queue.size();
  auto p = mean;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  period = p;
}

void ParaQueue::push(const cv::Mat &image) {
  // auto tt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  // std::cout <<"| "<< tt<<" |"<<std::endl;
  std::lock_guard<std::mutex> lock(m_mutex);
  update_times();
  m_queue.push(std::move(image));
  while (m_queue.size() > max_queue_size) {
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

ParaQueue::ParaQueue(int q_size) : max_queue_size(q_size) {}

double ParaQueue::get_fps() const {
  using namespace std::chrono_literals;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  return std::chrono::duration_cast<UNIT>(1s) / period;
  return period.count();
}
