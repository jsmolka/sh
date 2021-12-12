#pragma once

#include <array>
#include <functional>
#include <utility>

namespace sh {

namespace {

template <typename Callback, typename T>
concept make_array_callback = requires(Callback&& callback) {
  { std::invoke(std::forward<Callback>(callback), std::size_t{}) } -> std::same_as<T>;
};

}  // namespace

namespace detail {

template <typename T, std::size_t kSize, std::size_t... kSizes>
struct array {
  using type = std::array<typename array<T, kSizes...>::type, kSize>;
};

template <typename T, std::size_t kSize>
struct array<T, kSize> {
  using type = std::array<T, kSize>;
};

template <typename T, make_array_callback<T> Callback, std::size_t... kIs>
constexpr auto make_array(Callback&& callback, std::index_sequence<kIs...>)
    -> std::array<T, sizeof...(kIs)> {
  return {callback(kIs)...};
}

}  // namespace detail

template <typename T, std::size_t kSize, std::size_t... kSizes>
using array = typename detail::array<T, kSize, kSizes...>::type;

template <typename T, std::size_t kSize, make_array_callback<T> Callback>
constexpr auto make_array(Callback&& callback) -> std::array<T, kSize> {
  return detail::make_array<T>(std::forward<Callback>(callback), std::make_index_sequence<kSize>{});
}

}  // namespace sh
