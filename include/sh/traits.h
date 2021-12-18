#pragma once

#include <type_traits>

namespace sh {

template <typename T, typename... Ts>
inline constexpr bool is_any_of_v = std::disjunction_v<std::is_same<T, Ts>...>;

template <typename T, typename... Ts>
struct is_any_of : std::bool_constant<is_any_of_v<T, Ts...>> {};

template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = false;

template <template <typename...> typename Template, typename... Ts>
inline constexpr bool is_specialization_v<Template<Ts...>, Template> = true;

template <typename T, template <typename...> typename Template>
struct is_specialization : std::bool_constant<is_specialization_v<T, Template>> {};

template <typename T>
struct unqualified {
  using type = std::remove_cv_t<std::remove_pointer_t<std::decay_t<T>>>;
};

template <typename T>
using unqualified_t = typename unqualified<T>::type;

}  // namespace sh
