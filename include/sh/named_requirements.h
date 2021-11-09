#pragma once

#include <concepts>
#include <type_traits>

namespace sh {

namespace {

template <typename T, typename U>
concept Assignable = requires(T& t) {
  { t = std::declval<U>() } -> std::same_as<T&>;
};

}  // namespace

template <typename T>
concept DefaultConstructible = std::default_initializable<T>;

template <typename T>
concept MoveAssignable = Assignable<T, T&&> || Assignable<T, const T&>;

template <typename T>
concept CopyAssignable = MoveAssignable<T> && std::is_copy_assignable_v<T> && Assignable<T, T&> &&
    Assignable<T, const T&> && Assignable<T, const T&&>;

template <typename T>
concept MoveConstructible = std::constructible_from<T, T&&> || std::constructible_from<T, const T&>;

template <typename T>
concept CopyConstructible = MoveConstructible<T> && std::constructible_from<T, T&> &&
    std::constructible_from<T, const T&> && std::constructible_from<T, const T&&>;

struct x {
  x& operator=(x&&) = delete;
  x& operator=(const x&) = default;
  int y;
};

static_assert(MoveAssignable<x>);

}  // namespace sh
