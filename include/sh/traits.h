#pragma once

#include <type_traits>

namespace sh {

template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = false;

template <template <typename...> typename Template, typename... Ts>
inline constexpr bool is_specialization_v<Template<Ts...>, Template> = true;

template <typename T, template <typename...> typename Template>
struct is_specialization : std::bool_constant<is_specialization_v<T, Template>> {};

}  // namespace sh
