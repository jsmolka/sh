#pragma once

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

namespace sh {

template <std::input_iterator I, std::sentinel_for<I> S>
void destroy(I first, S last) {
  if constexpr (!std::is_trivially_destructible_v<std::iter_value_t<I>>) {
    for (; first != last; ++first) {
      std::destroy_at(std::to_address(first));
    }
  }
}

template <std::input_iterator I>
void destroy_n(I first, std::size_t count) {
  if constexpr (!std::is_trivially_destructible_v<std::iter_value_t<I>>) {
    for (; count; ++first, --count) {
      std::destroy_at(std::to_address(first));
    }
  }
}

template <std::input_iterator I, std::sentinel_for<I> S, std::forward_iterator D>
  requires std::constructible_from<std::iter_value_t<D>, std::iter_reference_t<I>>
auto uninitialized_copy(I first, S last, D dest) -> D {
  const auto rollback = dest;
  try {
    for (; first != last; ++first, ++dest) {
      std::construct_at(std::to_address(dest), *first);
    }
    return dest;
  } catch (...) {
    destroy(rollback, dest);
    throw;
  }
}

template <std::input_iterator I, std::forward_iterator D>
  requires std::constructible_from<std::iter_value_t<D>, std::iter_reference_t<I>>
auto uninitialized_copy_n(I first, std::size_t count, D dest) -> D {
  const auto rollback = dest;
  try {
    for (; count; ++first, --count, ++dest) {
      std::construct_at(std::to_address(dest), *first);
    }
    return dest;
  } catch (...) {
    destroy(rollback, dest);
    throw;
  }
}

template <std::input_iterator I, std::sentinel_for<I> S, std::forward_iterator D>
  requires std::constructible_from<std::iter_value_t<D>, std::iter_rvalue_reference_t<I>>
auto uninitialized_move(I first, S last, D dest) -> D {
  const auto rollback = dest;
  try {
    for (; first != last; ++first, ++dest) {
      std::construct_at(std::to_address(dest), std::move(*first));
    }
    return dest;
  } catch (...) {
    destroy(rollback, dest);
    throw;
  }
}

template <std::input_iterator I, std::forward_iterator D>
  requires std::constructible_from<std::iter_value_t<D>, std::iter_rvalue_reference_t<I>>
auto uninitialized_move_n(I first, std::size_t count, D dest) -> D {
  const auto rollback = dest;
  try {
    for (; count; ++first, --count, ++dest) {
      std::construct_at(std::to_address(dest), std::move(*first));
    }
    return dest;
  } catch (...) {
    destroy(rollback, dest);
    throw;
  }
}

template <std::forward_iterator I, std::sentinel_for<I> S, typename T>
  requires(std::constructible_from<std::iter_value_t<I>, const T&>)
auto uninitialized_fill(I first, S last, const T& value) -> I {
  const auto rollback = first;
  try {
    for (; first != last; ++first) {
      std::construct_at(std::to_address(first), value);
    }
    return first;
  } catch (...) {
    destroy(rollback, first);
    throw;
  }
}

template <std::forward_iterator I, typename T>
  requires(std::constructible_from<std::iter_value_t<I>, const T&>)
auto uninitialized_fill_n(I first, std::size_t count, const T& value) -> I {
  const auto rollback = first;
  try {
    for (; count; ++first, --count) {
      std::construct_at(std::to_address(first), value);
    }
    return first;
  } catch (...) {
    destroy(rollback, first);
    throw;
  }
}

template <std::forward_iterator I, std::sentinel_for<I> S>
  requires std::default_initializable<std::iter_value_t<I>>
void uninitialized_default_construct(I first, S last) {
  for (; first != last; ++first) {
    std::construct_at(std::to_address(first));
  }
}

template <std::forward_iterator I>
  requires std::default_initializable<std::iter_value_t<I>>
auto uninitialized_default_construct_n(I first, std::size_t count) -> I {
  for (; count; ++first, --count) {
    std::construct_at(std::to_address(first));
  }
  return first;
}

}  // namespace sh
