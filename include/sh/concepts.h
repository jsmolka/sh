#pragma once

#include <concepts>

namespace sh {

template <typename T>
concept copy_constructible = std::constructible_from<T, const T&>;

}  // namespace sh
