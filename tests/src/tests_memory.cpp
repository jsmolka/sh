#include "tests_memory.h"

#include <vector>

#include <sh/memory.h>

#include "ut.h"

void tests_uninitialized_copy() {
  "uninitalized_copy"_test = []() {
    std::vector<int> x{1, 2, 3};
    std::vector<int> y;
    y.reserve(3);
    sh::uninitialized_copy(x.begin(), x.end(), y.begin());
    expect(y[0] == 1);
    expect(y[1] == 2);
    expect(y[2] == 3);
  };
}

void tests_memory() {
  tests_uninitialized_copy();
}
