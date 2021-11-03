#include "tests_vector.h"

#include <sh/vector.h>

#include "ut.h"

auto sum(auto... args) { return (args + ...); }

void tests_vector() {
  "sum"_test = [] {
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 3) == 4_i);
  };
}
