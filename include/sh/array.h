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
constexpr auto make_array(Func&& func, std::index_sequence<kIs...>)
    -> std::array<T, sizeof...(kIs)> {
  return {func(kIs)...};
}

}  // namespace detail

template <typename T, std::size_t kSize, std::size_t... kSizes>
using array = typename detail::array<T, kSize, kSizes...>::type;

template <typename T, std::size_t kSize, typename Func>
  requires requires(Func&& f, std::size_t i) {
    { std::invoke(std::forward<Func>(f), i) } -> std::same_as<T>;
  }
constexpr auto make_array(Func&& func) -> std::array<T, kSize> {
  return detail::make_array<T>(std::forward<Func>(func), std::make_index_sequence<kSize>{});
}

}  // namespace sh
