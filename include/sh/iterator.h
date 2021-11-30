#pragma once

#include <utility>

#include <sh/concepts.h>

namespace sh {

template <typename T>
concept dereferencable = requires(T& t) {
  { *t } -> sh::different_from<void>;
};

template <dereferencable T>
using iter_reference_t = decltype(*std::declval<T&>());

}  // namespace sh
