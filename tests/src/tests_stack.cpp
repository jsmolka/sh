#include "tests_stack.h"

#include <sh/stack.h>

#include "types.h"
#include "ut.h"

namespace {

template <typename T, std::size_t kSize>
auto operator<<(std::ostream& out, const sh::stack<T, kSize>& stack) -> std::ostream& {
  out << "[";
  for (const auto& value : stack) {
    if (&value != stack.begin()) {
      out << ", ";
    }
    out << value;
  }
  out << "]";
  return out;
}

template <typename T, std::size_t N>
struct tests {
  using stack = sh::stack<T, N>;

  static auto test(std::string_view what) {
    return ::test("stack<{}, {}>::{}", typeid(T).name(), N, what);
  }
};

template <typename T, std::size_t N>
struct tests_accessors : tests<T, N> {
  using typename tests<T, N>::stack;
  using tests<T, N>::test;

  static void run() {
    test("top()") = []() {
      stack stk1{};
      stk1.push(0);
      stk1.push(1);

      expect(eq(stk1.top(), 1));
      expect(eq(const_cast<const stack&>(stk1).top(), 1));
    };

    test("peek()") = []() {
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

template <template <typename, std::size_t> typename Test, typename T>
void run() {
  Test<T, 0>::run();
  Test<T, 1>::run();
  Test<T, 2>::run();
  Test<T, 4>::run();
  Test<T, 8>::run();
}

template <template <typename, std::size_t> typename Test>
void run() {
  run<Test, int>();
  run<Test, test_type1>();
  run<Test, test_type2>();
  run<Test, test_type3>();
  run<Test, test_type4>();
}

}  // namespace

void tests_stack() {
  run<tests_accessors>();
}
