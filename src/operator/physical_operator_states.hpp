#pragma once
#include "operator_result_type.hpp"

namespace DaseX {

class GlobalSourceState {
public:
  int16_t parallel_nums; // 并行度
public:
  GlobalSourceState(int16_t parallel_nums_ = 1)
      : parallel_nums(parallel_nums_) {}

  virtual ~GlobalSourceState() {}

  virtual int16_t max_threads() { return parallel_nums; }

  template <typename TARGET> TARGET &Cast() {
    return reinterpret_cast<TARGET &>(*this);
  }
  template <typename TARGET> const TARGET &Cast() const {
    return reinterpret_cast<const TARGET &>(*this);
  }
};

class LocalSourceState {
public:
    virtual ~LocalSourceState() {}
    template <class TARGET>
    TARGET &Cast() {
        return reinterpret_cast<TARGET &>(*this);
    }
    template <class TARGET>
    const TARGET &Cast() const {
        return reinterpret_cast<const TARGET &>(*this);
    }
};

class GlobalOperatorState {
public:
  virtual ~GlobalOperatorState() {}

  template <typename TARGET> TARGET &Cast() {
    return reinterpret_cast<TARGET &>(*this);
  }
  template <typename TARGET> const TARGET &Cast() const {
    return reinterpret_cast<const TARGET &>(*this);
  }
};

class GlobalSinkState {
public:
  virtual ~GlobalSinkState() {}

  template <typename TARGET> TARGET &Cast() {
    return reinterpret_cast<TARGET &>(*this);
  }
  template <typename TARGET> const TARGET &Cast() const {
    return reinterpret_cast<const TARGET &>(*this);
  }
};

class LocalSinkState {
public:
  virtual ~LocalSinkState() {}

  template <typename TARGET> TARGET &Cast() {
    return reinterpret_cast<TARGET &>(*this);
  }
  template <typename TARGET> const TARGET &Cast() const {
    return reinterpret_cast<const TARGET &>(*this);
  }
};

struct OperatorSinkInput {
    GlobalSinkState &global_state;
    LocalSinkState &local_state;
};

struct OperatorSourceInput {
    GlobalSourceState &global_state;
    LocalSourceState &local_state;
};

} // namespace DaseX
