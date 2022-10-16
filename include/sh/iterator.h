#pragma once

#include <iterator>
#include <utility>
#include <sh/concepts.h>

namespace sh {

template<typename T>
using iterator_t = decltype(std::begin(std::declval<T&>()));

template<typename T>
using reverse_iterator_t = decltype(std::rbegin(std::declval<T&>()));

template<typename T>
using sentinel_t = decltype(std::end(std::declval<T&>()));

template<typename T>
using reverse_sentinel_t = decltype(std::rend(std::declval<T&>()));

template<typename T>
concept dereferencable = requires(T& t) {
  { *t } -> different_from<void>;
};

template<dereferencable T>
using iter_value_t = std::remove_reference_t<decltype(*std::declval<T&>())>;

template<dereferencable T>
using iter_reference_t = decltype(*std::declval<T&>());

template<typename I>
concept forward_iterator = requires(I& i) {
  requires dereferencable<I>;
  { ++i } -> std::same_as<I&>;
};

template<typename S, typename I>
concept sentinel_for = requires(I& i, S& s) {
  { i != s } -> std::same_as<bool>;
};

}  // namespace sh
