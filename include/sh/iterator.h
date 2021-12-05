#pragma once

#include <utility>

#include <sh/concepts.h>

namespace sh {

template <typename T>
using iterator_t = decltype(std::begin(std::declval<T&>()));

template <typename T>
using sentinel_t = decltype(std::end(std::declval<T&>()));

template <typename T>
concept dereferencable = requires(T& t) {
  { *t } -> sh::different_from<void>;
};

template <dereferencable T>
using iter_reference_t = decltype(*std::declval<T&>());

}  // namespace sh
