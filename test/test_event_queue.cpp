#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <thread>

#include "doctest.h"
#include "event_queue.hpp"
using namespace utility;

std::atomic_int32_t counter{0};

class echo_event : public event_handler {
 public:
  echo_event(std::string msg) { msg_ = std::move(msg); }

  virtual void process() override {
    std::cout << "echo: " << msg_ << std::endl;
    counter.fetch_add(1);
  }

 private:
  std::string msg_;
};

TEST_CASE("test event_queue") {
  event_queue eq;
  eq.enqueue(std::make_shared<echo_event>("hello"));
  eq.enqueue(std::make_shared<echo_event>("world"));

  auto fut = std::async([&eq]() {
    eq.enqueue(std::make_shared<echo_event>("async hello"));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    eq.enqueue(std::make_shared<echo_event>("async world"));
  });

  auto fut2 = std::async([&eq]() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    eq.stop();
  });

  eq.loop();
  fut.wait();
  fut2.wait();

  CHECK_EQ(counter.load(), 4);
}
