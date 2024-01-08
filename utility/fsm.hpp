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

#ifndef __UTILITY_FSM_HPP__
#define __UTILITY_FSM_HPP__

#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace utility {

namespace fsm {

template <typename FSM>
class state {
 public:
  using state_machine = FSM;
  using state_type = typename FSM::state_type;

  virtual void enter() = 0;
  virtual void exit() = 0;
  virtual bool transfer() = 0;

  virtual void set_fsm(state_machine *fsm) { fsm_ = fsm; }

 protected:
  state_machine *fsm_{nullptr};
};

class state_machine {
 public:
  using state_changed_notifier = std::function<void()>;

 public:
  virtual ~state_machine() {}

  virtual bool init() = 0;
  virtual void stop() = 0;

  virtual void set_state_changed_notifier(state_changed_notifier notifier) = 0;

  virtual bool trigger_state_transfer() = 0;
  virtual void async_trigger_state_transfer() = 0;

  virtual void log_state_changed(std::string cur_state_name,
                                 std::string next_state_name,
                                 const std::string &description) = 0;
};

template <typename FSM, typename StateType = int32_t>
class state_machine_impl : public state_machine {
 public:
  using state_type = StateType;
  using fsm_state = state<FSM>;

  ~state_machine_impl() { stop(); }

  state_type cur_state_type() {
    std::unique_lock<std::mutex> locker(state_mtx_);
    return cur_state_type_;
  }

  std::string cur_state_name() {
    std::unique_lock<std::mutex> locker(state_mtx_);
    return cur_state_name_;
  }

  std::shared_ptr<fsm_state> cur_state() {
    std::unique_lock<std::mutex> locker(state_mtx_);
    return cur_state_;
  }

  virtual void stop() override {
    if (trans_fut_.valid()) {
      std::cout << "wait trigger_state_transfer..." << std::endl;
      trans_fut_.wait();
    }

    std::unique_lock<std::mutex> locker(state_mtx_);
    if (cur_state_) {
      cur_state_->exit();
      cur_state_ = nullptr;
    }
    cur_state_name_ = "";
    cur_state_type_ = {};
  }

  virtual void set_state_changed_notifier(
      state_changed_notifier notifier) override {
    std::unique_lock<std::mutex> locker(state_mtx_);
    state_changed_notifier_ = notifier;
  }

  virtual bool trigger_state_transfer() override {
    auto state = cur_state();
    return state && state->transfer();
  }

  virtual void async_trigger_state_transfer() override {
    trans_fut_ = std::async([this]() {
      trigger_state_transfer();
    });
  }

  template <typename FsmState>
  bool is_state() {
    return cur_state_type() == FsmState::traits::type();
  }

  template <typename FsmState>
  void init_state() {
    change_state<FsmState>("init");
  }

  template <typename FsmState>
  void try_change_state(const std::string &description = "") {
    if (!is_state<FsmState>())
      change_state<FsmState>(description);
  }

  template <typename FsmState>
  void change_state(const std::string &description = "") {
    auto new_state_enum = FsmState::traits::type();
    auto new_state_name = FsmState::traits::name();

    auto old_state = cur_state();
    if (old_state)
      old_state->exit();

    log_state_changed(cur_state_name(), new_state_name, description);

    {
      // set new state
      std::unique_lock<std::mutex> locker(state_mtx_);
      cur_state_ = get_state<FsmState>();
      cur_state_name_ = new_state_name;
      cur_state_type_ = new_state_enum;
      cur_state_->set_fsm(get());
    }

    notify_state_changed();

    auto new_state = cur_state();
    new_state->enter();
  }

 private:
  FSM *get() { return static_cast<FSM *>(this); }

  // flyweight: if not exist, create it and return.
  template <typename FsmState>
  std::shared_ptr<fsm_state> get_state() {
    auto type = FsmState::traits::type();
    if (state_mgr_.count(type) == 0)
      state_mgr_[type] = FsmState::traits::creator();

    return state_mgr_[type];
  }

  void notify_state_changed() {
    state_changed_notifier notifier = nullptr;
    {
      std::unique_lock<std::mutex> locker(state_mtx_);
      notifier = state_changed_notifier_;
    }

    if (notifier)
      notifier();
  }

 private:
  std::future<void> trans_fut_;

  std::mutex state_mtx_;
  state_type cur_state_type_{};
  std::string cur_state_name_{"null"};
  std::shared_ptr<fsm_state> cur_state_{nullptr};
  state_changed_notifier state_changed_notifier_{nullptr};
  std::unordered_map<state_type, std::shared_ptr<fsm_state>> state_mgr_;
};

}  // namespace fsm

#define FSM_STATE_TRAITS(ClassName, StateType)                        \
  struct traits {                                                     \
    static constexpr decltype(StateType) type() { return StateType; } \
    static std::string name() { return #ClassName; }                  \
    static std::shared_ptr<ClassName> creator() {                     \
      return std::make_shared<ClassName>();                           \
    }                                                                 \
  }

}  // namespace utility

#endif  //__UTILITY_FSM_HPP__
