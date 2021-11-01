#pragma once

#include <cstddef>

namespace dz {

template <typename T, std::size_t kSize>
class vector {
 private:
  T stack_[kSize];
};

}  // namespace dz
