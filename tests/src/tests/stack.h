#pragma once

#include <sh/stack.h>

#include "types.h"
#include "ut.h"

template<typename T, std::size_t N>
auto operator<<(std::ostream& out, const sh::stack<T, N>& vector) -> std::ostream& {
  return out << fmt::format("[{}]", fmt::join(vector, ", "));
}

namespace tests_stack {

template<typename T, std::size_t N>
struct tests {
  using stack = sh::stack<T, N>;

  static auto test(std::string_view what) -> detail::test {
    return ::test("stack<{}, {}>::{}", typeid(T).name(), N, what);
  }
};

template<typename T, std::size_t N>
struct tests_accessors : tests<T, N> {
  using typename tests<T, N>::stack;
  using tests<T, N>::test;

  static void run() {
    test("top()") = [] {
      stack stk1{};
      stk1.push(0);
      stk1.push(1);

      expect(eq(stk1.top(), 1));
      expect(eq(const_cast<const stack&>(stk1).top(), 1));
    };

    test("peek()") = [] {
      stack stk1{};
      stk1.push(0);
      stk1.push(1);

      expect(eq(stk1.peek(0), 1));
      expect(eq(const_cast<const stack&>(stk1).peek(0), 1));
      expect(eq(stk1.peek(1), 0));
      expect(eq(const_cast<const stack&>(stk1).peek(1), 0));
    };
  }
};

template<typename T, std::size_t N>
struct tests_push : tests<T, N> {
  using typename tests<T, N>::stack;
  using tests<T, N>::test;

  static void run() {
    test("push(const value_type&)") = [] {
      if constexpr (sh::copy_constructible<T>) {
        T v1{0};
        T v2{1};
        T v3{2};

        stack stk1{};
        stk1.push(v1);
        stk1.push(v2);
        stk1.push(v3);
        expect(eq(stk1, stack{0, 1, 2}));
      }
    };

    test("push(value_type&&)") = [] {
      if constexpr (sh::move_constructible<T>) {
        T v1{0};
        T v2{1};
        T v3{2};

        stack stk1{};
        stk1.push(std::move(v1));
        stk1.push(std::move(v2));
        stk1.push(std::move(v3));
        expect(eq(stk1[0], 0));
        expect(eq(stk1[1], 1));
        expect(eq(stk1[2], 2));
      }
    };
  }
};

template<typename T, std::size_t N>
struct tests_emplace : tests<T, N> {
  using typename tests<T, N>::stack;
  using tests<T, N>::test;

  static void run() {
    test("emplace(Args&&)") = [] {
      stack stk1{};
      expect(eq(stk1.emplace(0), 0));
      expect(eq(stk1.emplace(1), 1));
      expect(eq(stk1.emplace(2), 2));
      expect(eq(stk1[0], 0));
      expect(eq(stk1[1], 1));
      expect(eq(stk1[2], 2));
    };
  }
};

template<typename T, std::size_t N>
struct tests_pop : tests<T, N> {
  using typename tests<T, N>::stack;
  using tests<T, N>::test;

  static void run() {
    test("pop()") = [] {
      stack stk1{};
      stk1.emplace(0);
      stk1.emplace(1);
      expect(eq(stk1.size(), 2));
      stk1.pop();
      expect(eq(stk1.size(), 1));
      stk1.pop();
      expect(eq(stk1.size(), 0));
    };

    test("pop(size_type)") = [] {
      stack stk1{};
      stk1.emplace(0);
      stk1.emplace(1);
      stk1.emplace(2);
      expect(eq(stk1.size(), 3));
      stk1.pop(2);
      expect(eq(stk1.size(), 1));
      stk1.pop(1);
      expect(eq(stk1.size(), 0));
    };
  }
};

template<typename T, std::size_t N>
struct tests_pop_value : tests<T, N> {
  using typename tests<T, N>::stack;
  using tests<T, N>::test;

  static void run() {
    test("pop_value()") = [] {
      if constexpr (sh::move_constructible<T>) {
        stack stk1{};
        stk1.emplace(0);
        stk1.emplace(1);
        expect(eq(stk1.pop_value(), 1));
        expect(eq(stk1.pop_value(), 0));
      }
    };
  }
};

template<template<typename, std::size_t> typename Test, typename T>
void run() {
  Test<T, 0>::run();
  Test<T, 1>::run();
  Test<T, 2>::run();
  Test<T, 4>::run();
  Test<T, 8>::run();
}

template<template<typename, std::size_t> typename Test>
void run() {
  run<Test, int>();
  run<Test, type1>();
  run<Test, type2>();
  run<Test, type3>();
  run<Test, type4>();
}

inline suite _ = [] {
  run<tests_accessors>();
  run<tests_push>();
  run<tests_emplace>();
  run<tests_pop>();
  run<tests_pop_value>();
};

}  // namespace tests_stack
