#include "tests_array.h"

#include <sh/array.h>

#include "ut.h"

void tests_array() {
  sh::array<int, 2, 2> arr = {{{1, 2}, {3, 4}}};
  expect(eq(arr[0][0], 1));
  expect(eq(arr[0][1], 2));
  expect(eq(arr[1][0], 3));
  expect(eq(arr[1][1], 4));

  static constexpr auto kArr = sh::make_array<int, 4>([](std::size_t index) -> int {
    return 2 * index;
  });
  expect(eq(kArr[0], 0));
  expect(eq(kArr[1], 2));
  expect(eq(kArr[2], 4));
  expect(eq(kArr[3], 6));
}
