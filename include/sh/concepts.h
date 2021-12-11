#pragma once

#include <concepts>
#include <type_traits>

namespace sh {

template <typename T>
concept copy_constructible = std::constructible_from<T, const T&>;

template <typename T>
concept copy_assignable = std::assignable_from<T&, const T&>;

template <typename T>
concept move_constructible = std::constructible_from<T, T&&>;

template <typename T>
concept move_assignable = std::assignable_from<T&, T&&>;

template <typename T, typename U>
concept different_from = !std::same_as<T, U>;

template <typename T, typename... Ts>
concept any_of = std::disjunction_v<std::is_same<T, Ts>...>;

template <typename... Ts>
concept not_empty = sizeof...(Ts) > 0;

}  // namespace sh
