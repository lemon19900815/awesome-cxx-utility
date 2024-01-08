/*

Copyright (c) 2024 lemon19900815@buerjia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#ifndef __UTILITY_EVENT_QUEUE_HPP__
#define __UTILITY_EVENT_QUEUE_HPP__

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

#define USING_PTR(Type)              \
  using ptr = std::shared_ptr<Type>; \
  using wptr = std::weak_ptr<Type>;

namespace utility {

class event_handler {
 public:
  USING_PTR(event_handler);
  virtual ~event_handler() = default;
  virtual void process() = 0;
};

class event_queue final {
 public:
  ~event_queue() { stop(); }

  void enqueue(event_handler::ptr ev_handler) {
    std::lock_guard<std::mutex> guard(mtx_);
    queue_.emplace_back(std::move(ev_handler));
    cv_.notify_one();
  }

  void stop() {
    stop_.store(true);
    cv_.notify_all();
  }

  void loop() {
    while (!stop_.load()) {
      auto cur_event = dequeue();
      if (cur_event) {
        cur_event->process();
      }
    }

    std::lock_guard<std::mutex> guard(mtx_);
    queue_.clear();
  }

 private:
  event_handler::ptr dequeue() {
    event_handler::ptr cur_event = nullptr;

    {
      std::unique_lock<std::mutex> lock(mtx_);
      cv_.wait(lock, [this]() {
        return stop_.load() || !queue_.empty();
      });

      if (stop_.load()) {
        return nullptr;
      }

      cur_event = std::move(queue_.front());
      queue_.pop_front();
    }

    return cur_event;
  }

 private:
  std::deque<event_handler::ptr> queue_;
  std::atomic_bool stop_{false};

  std::mutex mtx_;
  std::condition_variable cv_;
};

}  // namespace utility

#endif  // __UTILITY_EVENT_QUEUE_HPP__
