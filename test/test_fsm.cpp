#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "fsm.hpp"
using namespace utility;

constexpr int32_t kStateReady = 1;
constexpr int32_t kStateRunning = 2;
constexpr int32_t kStateExit = 3;

class test_state_mgr : public fsm::state_machine_impl<test_state_mgr> {
 public:
  virtual bool init() override { return true; }

  virtual void log_state_changed(std::string cur_state_name,
                                 std::string next_state_name,
                                 const std::string& description) override {
    std::cout << cur_state_name << "->" << next_state_name << ": "
              << description << std::endl;
  }
};

class ready_state;
class running_state;
class exit_state;

class ready_state : public fsm::state<test_state_mgr> {
 public:
  FSM_STATE_TRAITS(ready_state, kStateReady);

  virtual void enter() override { std::cout << __FUNCTION__ << std::endl; }

  virtual void exit() override { std::cout << __FUNCTION__ << std::endl; }

  virtual bool transfer() override {
    std::cout << __FUNCTION__ << std::endl;
    fsm_->change_state<running_state>();
    return true;
  }
};

class running_state : public fsm::state<test_state_mgr> {
 public:
  FSM_STATE_TRAITS(running_state, kStateRunning);

  virtual void enter() override { std::cout << __FUNCTION__ << std::endl; }

  virtual void exit() override { std::cout << __FUNCTION__ << std::endl; }

  virtual bool transfer() override {
    std::cout << __FUNCTION__ << std::endl;
    fsm_->change_state<exit_state>();
    return true;
  }
};

class exit_state : public fsm::state<test_state_mgr> {
 public:
  FSM_STATE_TRAITS(exit_state, kStateExit);

  virtual void enter() override { std::cout << __FUNCTION__ << std::endl; }

  virtual void exit() override { std::cout << __FUNCTION__ << std::endl; }

  virtual bool transfer() override {
    std::cout << __FUNCTION__ << std::endl;
    // fsm_->change_state<exit_state>();
    return false;
  }
};

TEST_CASE("test fsm") {
  test_state_mgr sm;
  CHECK_EQ(sm.init(), true);
  CHECK_EQ(sm.cur_state(), nullptr);
  CHECK_EQ(sm.cur_state_type(), 0);
  CHECK_EQ(sm.cur_state_name(), std::string("null"));

  sm.set_state_changed_notifier([&sm]() {
    std::cout << "cur state: " << sm.cur_state_name()
              << ", type: " << sm.cur_state_type() << std::endl;
  });

  sm.init_state<ready_state>();
  CHECK_EQ(sm.cur_state_type(), kStateReady);

  CHECK_EQ(sm.trigger_state_transfer(), true);
  CHECK_EQ(sm.cur_state_type(), kStateRunning);

  CHECK_EQ(sm.trigger_state_transfer(), true);
  CHECK_EQ(sm.cur_state_type(), kStateExit);

  // exit_state::transfer返回false
  CHECK_EQ(sm.trigger_state_transfer(), false);

  sm.stop();
  CHECK_EQ(sm.cur_state_type(), 0);
}
