#pragma once

#include <utility>

namespace sh {

template <typename Function>
class scope_guard {
 public:
  scope_guard(Function&& function) noexcept : function_(std::move(function)) {}

  ~scope_guard() {
    if (function_) {
      function_();
    }
  }

  void release() {
    function_ = nullptr;
  }

 private:
  Function function_;
};

}  // namespace sh
