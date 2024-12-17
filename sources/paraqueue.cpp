#include "paraqueue.hpp"
#include "debug_log.hpp"

#include <iostream>

#define TIME_QUEUE_LENGTH 30
void ParaQueue::update_times()
{
  auto now = CLOCK::now();
  m_time_queue.push(now);
  if (m_time_queue.size() < 4)
  {
    return;
  }
  while (m_time_queue.size() > TIME_QUEUE_LENGTH)
  {
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

UNIT get_elapsed(std::queue<TIMEPOINT> const &times)
{
  if (times.empty())
  {
    return UNIT(99999999);
  }
  auto now = CLOCK::now();
  auto diff = now - times.back();
  return diff;
}

void ParaQueue::push(const avl::Image &image)
{
  auto elapsed = get_elapsed(m_time_queue);
  auto min_period = std::chrono::seconds(1) / m_max_allowed_frequency_hz;
  if (false && m_max_allowed_frequency_hz > 0)
  {
    // if max_freq is set, nmake sure new frames wont arrive too fast.
    if (elapsed < min_period*0.9)
    {
      return;
    }
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  update_times();
  m_queue.push(std::move(image));
  while (m_queue.size() > m_max_queue_size)
  {
    m_queue.pop();
  }
}
bool ParaQueue::pop(avl::Image &image)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_queue.empty())
    return false;
  
  image = m_queue.front();
  m_queue.pop();
  return true;
}

bool ParaQueue::has_enqueued() const { return !m_queue.empty(); }
bool ParaQueue::is_empty() const { return m_queue.empty(); }

ParaQueue::ParaQueue() : m_max_allowed_frequency_hz(-1), m_max_queue_size(16), m_period(99999999) {}
ParaQueue::ParaQueue(double max_freq) : m_max_allowed_frequency_hz(max_freq), m_max_queue_size(16), m_period(99999999) {}

double ParaQueue::get_fps() const
{
  using namespace std::chrono_literals;
  std::lock_guard<std::mutex> period_lock(m_period_mutex);
  return std::chrono::duration_cast<UNIT>(1s) / m_period;
  return m_period.count();
}

void ParaQueue::set_max_frequency_hz(double max_freq)
{
  m_max_allowed_frequency_hz = max_freq;
}

TIMEPOINT ParaQueue::last_frame_time()
{
  if (m_time_queue.empty())
  {
    return TIMEPOINT();
  }
  return m_time_queue.back();
}
