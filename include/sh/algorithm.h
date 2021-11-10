#pragma once

#include <iterator>
#include <utility>

namespace sh {

template <std::input_iterator I, std::sentinel_for<I> S, std::forward_iterator D>
  requires(std::indirectly_copyable<I, D>)
auto copy(I first, S last, D dest) -> D {
  for (; first != last; ++first, ++dest) {
    *dest = *first;
  }
  return dest;
}

template <std::input_iterator I, std::forward_iterator D>
  requires(std::indirectly_copyable<I, D>)
auto copy_n(I first, std::size_t count, D dest) -> D {
  for (; count; ++first, --count, ++dest) {
    *dest = *first;
  }
  return dest;
}

template <std::input_iterator I, std::sentinel_for<I> S, std::forward_iterator D>
  requires(std::indirectly_movable<I, D>)
auto move(I first, S last, D dest) -> D {
  for (; first != last; ++first, ++dest) {
    *dest = std::move(*first);
  }
  return dest;
}

template <std::input_iterator I, std::forward_iterator D>
  requires(std::indirectly_movable<I, D>)
auto move_n(I first, std::size_t count, D dest) -> D {
  for (; count; ++first, --count, ++dest) {
    *dest = std::move(*first);
  }
  return dest;
}

}  // namespace sh
