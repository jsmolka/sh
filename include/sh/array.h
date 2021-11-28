#pragma once

#include <array>
#include <functional>
#include <utility>

namespace sh {

namespace detail {

template <typename T, std::size_t kSize, std::size_t... kSizes>
struct array {
  using type = std::array<typename array<T, kSizes...>::type, kSize>;
};

template <typename T, std::size_t kSize>
struct array<T, kSize> {
  using type = std::array<T, kSize>;
};

template <typename T, typename Func, std::size_t... kIs>
constexpr std::array<T, sizeof...(kIs)> make_array(Func&& func, std::index_sequence<kIs...>) {
  return {func(kIs)...};
}

}  // namespace detail

template <typename T, std::size_t kSize, std::size_t... kSizes>
using array = typename detail::array<T, kSize, kSizes...>::type;

template <typename T, std::size_t kSize, typename Func>
  requires requires(Func&& f) {
    { std::invoke(std::forward<Func>(f), std::size_t{}) } -> std::same_as<T>;
  }
constexpr std::array<T, kSize> make_array(Func&& func) {
  return detail::make_array<T>(std::forward<Func>(func), std::make_index_sequence<kSize>{});
}

}  // namespace sh
