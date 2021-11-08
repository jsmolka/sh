#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>

namespace sh {

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
