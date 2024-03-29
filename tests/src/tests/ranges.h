#pragma once

#include <vector>
#include <list>

#include <sh/ranges.h>

#include "ut.h"

namespace tests_ranges {

inline suite _ = [] {
  "enumerate<vector>"_test = [] {
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

  "enumerate<array>"_test = [] {
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

  "enumerate<list>"_test = [] {
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

  "enumerate nested"_test = [] {
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

  "reversed<vector>"_test = [] {
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

  "reversed<array>"_test = [] {
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

  "reversed<list>"_test = [] {
    std::list<int> x = {0, 1, 2};

    std::size_t i = 2;
    for (const auto& value : sh::reversed(x)) {
      expect(eq(value, i--));
    }
  };

  "contains<vector>"_test = [] {
    std::vector<int> x = {0, 1, 2};
    expect(sh::contains(x, 0));
    expect(sh::contains(x, 1));
    expect(sh::contains(x, 2));
    expect(!sh::contains(x, 3));
  };

  "contains<array>"_test = [] {
    int x[] = {0, 1, 2};
    expect(sh::contains(x, 0));
    expect(sh::contains(x, 1));
    expect(sh::contains(x, 2));
    expect(!sh::contains(x, 3));
  };

  "contains<list>"_test = [] {
    std::list<int> x = {0, 1, 2};
    expect(sh::contains(x, 0));
    expect(sh::contains(x, 1));
    expect(sh::contains(x, 2));
    expect(!sh::contains(x, 3));
  };
};

}  // namespace tests_ranges
