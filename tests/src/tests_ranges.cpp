#include "tests_ranges.h"

#include <list>
#include <vector>

#include <sh/ranges.h>

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

  ::test("nested enumerate") = []() {
    std::vector<int> x = {0, 1, 2};
    auto range = sh::enumerate(x, 1);

    std::size_t i = 0;
    for (auto [index, value] : sh::enumerate(range, 2)) {
      expect(eq(index, i + 2));
      expect(eq(value.index, i + 1));
      expect(eq(value.value, i));
      value.value *= 2;
      i++;
    }
    expect(eq(x[0], 0));
    expect(eq(x[1], 2));
    expect(eq(x[2], 4));
  };
}

void tests_reverse() {
  ::test("reversed<vector>") = []() {
    std::vector<int> x = {0, 1, 2};

    std::size_t i = 2;
    for (const auto& value : sh::reversed(x)) {
      expect(eq(value, i--));
    }

    for (auto& value : sh::reversed(x)) {
      value++;
    }

    expect(eq(x[0], 1));
    expect(eq(x[1], 2));
    expect(eq(x[2], 3));
  };

  ::test("reversed<array>") = []() {
    int x[] = {0, 1, 2};

    std::size_t i = 2;
    for (const auto& value : sh::reversed(x)) {
      expect(eq(value, i--));
    }

    for (auto& value : sh::reversed(x)) {
      value++;
    }

    expect(eq(x[0], 1));
    expect(eq(x[1], 2));
    expect(eq(x[2], 3));
  };

  ::test("reversed<list>") = []() {
    std::list<int> x = {0, 1, 2};

    std::size_t i = 2;
    for (const auto& value : sh::reversed(x)) {
      expect(eq(value, i--));
    }
  };
}

}  // namespace

void tests_ranges() {
  tests_enumerate();
  tests_reverse();
}
