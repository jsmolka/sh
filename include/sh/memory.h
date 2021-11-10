#pragma once

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

namespace sh {

template <std::input_iterator I, std::sentinel_for<I> S, typename A>
void destroy_a(I first, S last, A& alloc) {
  using Allocator = std::allocator_traits<A>;
  if constexpr (!std::is_trivially_destructible_v<std::iter_value_t<I>>) {
    for (; first != last; ++first) {
      Allocator::destroy(alloc, std::to_address(first));
    }
  }
}

template <std::input_iterator I, typename A>
void destroy_n_a(I first, std::size_t count, A& alloc) {
  using Allocator = std::allocator_traits<A>;
  if constexpr (!std::is_trivially_destructible_v<std::iter_value_t<I>>) {
    for (; count; ++first, --count) {
      Allocator::destroy(alloc, std::to_address(first));
    }
  }
}

template <std::input_iterator I, std::sentinel_for<I> S, std::forward_iterator D, typename A>
  requires(std::constructible_from<std::iter_value_t<D>, std::iter_reference_t<I>>)
auto uninitialized_copy_a(I first, S last, D dest, A& alloc) -> D {
  using Allocator = std::allocator_traits<A>;
  auto current = dest;
  try {
    for (; current != last; ++first, ++current) {
      Allocator::construct(alloc, std::to_address(current), *first);
    }
    return current;
  } catch (...) {
    destroy_a(dest, current, alloc);
    throw;
  }
}

template <std::input_iterator I, std::forward_iterator D, typename A>
  requires(std::constructible_from<std::iter_value_t<D>, std::iter_reference_t<I>>)
auto uninitialized_copy_n_a(I first, std::size_t count, D dest, A& alloc) -> D {
  using Allocator = std::allocator_traits<A>;
  auto current = dest;
  try {
    for (; count; ++first, --count, ++current) {
      Allocator::construct(alloc, std::to_address(current), *first);
    }
    return current;
  } catch (...) {
    destroy_a(dest, current, alloc);
    throw;
  }
}

template <std::input_iterator I, std::sentinel_for<I> S, std::forward_iterator D, typename A>
  requires(std::constructible_from<std::iter_value_t<D>, std::iter_rvalue_reference_t<I>>)
auto uninitialized_move_a(I first, S last, D dest, A& alloc) -> D {
  using Allocator = std::allocator_traits<A>;
  auto current = dest;
  try {
    for (; current != last; ++first, ++current) {
      Allocator::construct(alloc, std::to_address(current), std::move(*first));
    }
    return current;
  } catch (...) {
    destroy_a(dest, current, alloc);
    throw;
  }
}

template <std::input_iterator I, std::forward_iterator D, typename A>
  requires(std::constructible_from<std::iter_value_t<D>, std::iter_rvalue_reference_t<I>>)
auto uninitialized_move_n_a(I first, std::size_t count, D dest, A& alloc) -> D {
  using Allocator = std::allocator_traits<A>;
  auto current = dest;
  try {
    for (; count; ++first, --count, ++current) {
      Allocator::construct(alloc, std::to_address(current), std::move(*first));
    }
    return current;
  } catch (...) {
    destroy_a(dest, current, alloc);
    throw;
  }
}

template <std::forward_iterator I, std::sentinel_for<I> S, typename T, typename A>
  requires(std::constructible_from<std::iter_value_t<I>, const T&>)
auto uninitialized_fill_a(I first, S last, const T& value, A& alloc) -> I {
  using Allocator = std::allocator_traits<A>;
  auto current = first;
  try {
    for (; current != last; ++current) {
      Allocator::construct(alloc, std::to_address(current), value);
    }
    return current;
  } catch (...) {
    destroy_a(first, current, alloc);
    throw;
  }
}

template <std::forward_iterator I, typename T, typename A>
  requires(std::constructible_from<std::iter_value_t<I>, const T&>)
auto uninitialized_fill_n_a(I first, std::size_t count, const T& value, A& alloc) -> I {
  using Allocator = std::allocator_traits<A>;
  auto current = first;
  try {
    for (; count; ++current, --count) {
      Allocator::construct(alloc, std::to_address(current), value);
    }
    return current;
  } catch (...) {
    destroy_a(first, current, alloc);
    throw;
  }
}

template <std::forward_iterator I, std::sentinel_for<I> S, typename A>
  requires(std::default_initializable<std::iter_value_t<I>>)
void uninitialized_default_construct_a(I first, S last, A& alloc) {
  using Allocator = std::allocator_traits<A>;
  if constexpr (!std::is_trivially_default_constructible_v<std::iter_value_t<I>>) {
    for (; first != last; ++first) {
      Allocator::construct(alloc, std::to_address(first));
    }
  }
}

template <std::forward_iterator I, typename A>
  requires(std::default_initializable<std::iter_value_t<I>>)
auto uninitialized_default_construct_n_a(I first, std::size_t count, A& alloc) -> I {
  using Allocator = std::allocator_traits<A>;
  if constexpr (!std::is_trivially_default_constructible_v<std::iter_value_t<I>>) {
    for (; count; ++first, --count) {
      Allocator::construct(alloc, std::to_address(first));
    }
  }
  return first;
}

namespace {
template <typename InputIt, typename OutputIt>
concept same_contiguous_memory = requires {
  std::contiguous_iterator<InputIt>;
  std::contiguous_iterator<OutputIt>;
  std::is_same_v<std::iter_value_t<InputIt>, std::iter_value_t<OutputIt>>;
};

template <typename T>
concept memory_copy_constructible = requires {
  std::is_trivially_copy_constructible_v<T>;
  std::is_nothrow_copy_constructible_v<T>;
};

template <typename T>
concept memory_copy_assignable = requires {
  std::is_trivially_copy_assignable_v<T>;
  std::is_nothrow_copy_assignable_v<T>;
};

template <typename T>
concept memory_move_constructible = requires {
  std::is_trivially_move_constructible_v<T>;
  std::is_nothrow_move_constructible_v<T>;
};

template <typename T>
concept memory_move_assignable = requires {
  std::is_trivially_move_assignable_v<T>;
  std::is_nothrow_move_assignable_v<T>;
};

}  // namespace

template <typename InputIt, typename OutputIt>
  requires(same_contiguous_memory<InputIt, OutputIt>)
auto memcpy_n(InputIt first, std::size_t count, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  std::memcpy(std::to_address(dest), std::to_address(first), sizeof(T) * count);
  return dest + count;
}

template <typename InputIt, typename OutputIt>
  requires(same_contiguous_memory<InputIt, OutputIt>)
auto memcpy(InputIt first, InputIt last, OutputIt dest) -> OutputIt {
  return sh::memcpy_n(first, std::distance(first, last), dest);
}

template <typename InputIt, typename OutputIt>
auto copy_n(InputIt first, std::size_t count, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_copy_assignable<T>) {
  //  return sh::memcpy_n(first, count, dest);
  //} else {
  return std::copy_n(first, count, dest);
  //}
}

template <typename InputIt, typename OutputIt>
auto copy(InputIt first, InputIt last, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_copy_assignable<T>) {
  //  return sh::memcpy(first, last, dest);
  //} else {
  return std::copy(first, last, dest);
  //}
}

template <typename InputIt, typename OutputIt>
auto uninitialized_copy_n(InputIt first, std::size_t count, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_copy_constructible<T>) {
  //  return sh::memcpy_n(first, count, dest);
  //} else if constexpr (std::is_nothrow_copy_constructible_v<T>) {
  //  for (; count; ++first, --count, ++dest) {
  //    std::construct_at(dest, *first);
  //  }
  //  return dest;
  //} else {
  return std::uninitialized_copy_n(first, count, dest);
  //}
}

template <typename InputIt, typename OutputIt>
auto uninitialized_copy(InputIt first, InputIt last, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_copy_constructible<T>) {
  //  return sh::memcpy(first, last, dest);
  //} else if constexpr (std::is_nothrow_copy_constructible_v<T>) {
  //  for (; first != last; ++first, ++dest) {
  //    std::construct_at(dest, *first);
  //  }
  //  return dest;
  //} else {
  return std::uninitialized_copy(first, last, dest);
  //}
}

template <typename InputIt, typename OutputIt>
  requires(same_contiguous_memory<InputIt, OutputIt>)
auto memmov_n(InputIt first, std::size_t count, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  std::memmove(std::to_address(dest), std::to_address(first), sizeof(T) * count);
  return dest + count;
}

template <typename InputIt, typename OutputIt>
  requires(same_contiguous_memory<InputIt, OutputIt>)
auto memmov(InputIt first, InputIt last, OutputIt dest) -> OutputIt {
  return sh::memmov_n(first, std::distance(first, last), dest);
}

template <typename InputIt, typename OutputIt>
auto move_n(InputIt first, std::size_t count, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_move_assignable<T>) {
  //  return sh::memmov_n(first, count, dest);
  //} else {
  return std::copy_n(std::make_move_iterator(first), count, dest);
  //}
}

template <typename InputIt, typename OutputIt>
auto move(InputIt first, InputIt last, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_move_assignable<T>) {
  //  return sh::memmov(first, last, dest);
  //} else {
  return std::move(first, last, dest);
  //}
}

template <typename InputIt, typename OutputIt>
auto uninitialized_move_n(InputIt first, std::size_t count, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_move_constructible<T>) {
  //  return sh::memmov(first, count, dest);
  //} else if constexpr (std::is_nothrow_move_constructible_v<T>) {
  //  for (; count; ++first, --count, ++dest) {
  //    std::construct_at(dest, std::move(*first));
  //  }
  //  return dest;
  //} else {
  return std::uninitialized_move_n(first, count, dest);
  //}
}

template <typename InputIt, typename OutputIt>
auto uninitialized_move(InputIt first, InputIt last, OutputIt dest) -> OutputIt {
  using T = std::iter_value_t<OutputIt>;
  // if constexpr (same_contiguous_memory<InputIt, OutputIt> && memory_move_constructible<T>) {
  //  return sh::memmov(first, last, dest);
  //} else if constexpr (std::is_nothrow_move_constructible_v<T>) {
  //  for (; first != last; ++first, ++dest) {
  //    std::construct_at(dest, std::move(*first));
  //  }
  //  return dest;
  //} else {
  return std::uninitialized_move(first, last, dest);
  //}
}

}  // namespace sh
