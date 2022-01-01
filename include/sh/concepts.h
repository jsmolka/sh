#pragma once

#include <concepts>

#include <sh/traits.h>

namespace sh {

template<typename T>
concept copy_constructible = std::constructible_from<T, const T&>;

template<typename T>
concept copy_assignable = std::assignable_from<T&, const T&>;

template<typename T>
concept move_constructible = std::constructible_from<T, T&&>;

template<typename T>
concept move_assignable = std::assignable_from<T&, T&&>;

template<typename T, typename U>
concept different_from = !std::same_as<T, U>;

template<typename T, template<typename...> typename Template>
concept specialization = is_specialization_v<T, Template>;

template<typename T, typename... Ts>
concept any_of = is_any_of_v<T, Ts...>;

template<typename... Ts>
concept not_empty = sizeof...(Ts) > 0;

}  // namespace sh
