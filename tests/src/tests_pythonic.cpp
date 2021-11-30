#include "tests_pythonic.h"

#include <list>
#include <vector>

#include <sh/pythonic.h>

#include "ut.h"

namespace {

void tests_enumerate() {
  ::test("enumerate<vector>") = []() {
    std::vector<int> x = {1, 2, 3};

    std::size_t i = 0;
    for (const auto& [index, value] : sh::enumerate(x)) {
      expect(eq(index, i++));
      expect(eq(value, i));
    }
    expect(eq(i, 3));

    for (auto [index, value] : sh::enumerate(x)) {
      value++;
    }

    expect(eq(x[0], 2));
    expect(eq(x[1], 3));
    expect(eq(x[2], 4));
  };

  ::test("enumerate<array>") = []() {
    int x[] = {1, 2, 3};

    std::size_t i = 0;
    for (const auto& [index, value] : sh::enumerate(x)) {
      expect(eq(index, i++));
      expect(eq(value, i));
    }
    expect(eq(i, 3));

    for (auto [index, value] : sh::enumerate(x)) {
      value++;
    }

    expect(eq(x[0], 2));
    expect(eq(x[1], 3));
    expect(eq(x[2], 4));
  };

  ::test("enumerate<list>") = []() {
    std::list<int> x = {1, 2, 3};

    std::size_t i = 0;
    for (const auto& [index, value] : sh::enumerate(x)) {
      expect(eq(index, i++));
      expect(eq(value, i));
    }
    expect(eq(i, 3));

    for (auto [index, value] : sh::enumerate(x)) {
      value++;
    }
  };
}

}  // namespace

void tests_pythonic() {
  tests_enumerate();
}
