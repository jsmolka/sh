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
  };

  ::test("enumerate<array>") = []() {
    int x[] = {1, 2, 3};

    std::size_t i = 0;
    for (const auto& [index, value] : sh::enumerate(x)) {
      expect(eq(index, i++));
      expect(eq(value, i));
    }
    expect(eq(i, 3));
  };

  ::test("enumerate<list>") = []() {
    std::list<int> x = {1, 2, 3};

    std::size_t i = 0;
    for (const auto& [index, value] : sh::enumerate(x)) {
      expect(eq(index, i++));
      expect(eq(value, i));
    }
    expect(eq(i, 3));
  };
}

}  // namespace

void tests_pythonic() {
  tests_enumerate();
}
