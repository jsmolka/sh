#pragma once

#include <concepts>

namespace sh {

template <typename T>
concept copy_constructible = std::constructible_from<T, const T&>;

template <typename T>
concept copy_assignable = std::assignable_from<T&, const T&>;

template <typename T>
concept move_constructible = std::constructible_from<T, T&&>;

template <typename T>
concept move_assignable = std::assignable_from<T&, T&&>;

}  // namespace sh
