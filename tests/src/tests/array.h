#pragma once

#include <sh/array.h>

#include "ut.h"

namespace tests_array {

inline suite _ = [] {
  "array"_test = [] {
    sh::array<int, 2, 2> array = {{{1, 2}, {3, 4}}};
    expect(eq(array[0][0], 1));
    expect(eq(array[0][1], 2));
    expect(eq(array[1][0], 3));
    expect(eq(array[1][1], 4));
  };

  "make_array"_test = [] {
    static constexpr auto kArray = sh::make_array<int, 4>([](std::size_t index) -> int {
      return 2 * index;
    });
    expect(eq(kArray[0], 0));
    expect(eq(kArray[1], 2));
    expect(eq(kArray[2], 4));
    expect(eq(kArray[3], 6));
  };
};

}  // namespace tests_array
