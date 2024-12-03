#include "paraqueue.hpp"
#include "debug_log.hpp"
#include <AVLConverters/AVL_OpenCV.h>
#include <iostream>

void update_times(
    std::queue<std::chrono::time_point<CLOCK>> &queue,
    std::atomic<std::chrono::duration<std::chrono::milliseconds>> &period) {
  auto now = CLOCK::now();
  queue.push(now);
  while(queue.size()>100){
    queue.pop();
  }
  auto oldest = queue.back();
  auto newest = queue.front();
  auto diff = newest - oldest;
  auto mean = diff.count() / queue.size();
  auto  p  = std::chrono::duration_cast<std::chrono::milliseconds>(mean);
}

void ParaQueue::push(cv::Mat const &image) {
  std::lock_guard<std::mutex> lock(m_mutex);
  avl::Image avlimg;
  avl::CVMatToAvlImage_Switched(image, avlimg);
  m_queue.push(avlimg);
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
