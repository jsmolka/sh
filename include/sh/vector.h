#pragma once

#include <cstddef>

namespace sh {

template <typename T, std::size_t kSize = 0>
class vector {
 private:
  T stack_[kSize];
};

template <typename T>
class vector<T, 0> {
 private:
  T* data_;
  T* head_;
  T* last_;
};

}  // namespace sh
