#include "tests_vector.h"

#include <sh/vector.h>

#include <iostream>
#include <memory>

#include "ut.h"

void tests_vector() {
  sh::vector<int> x;
  x.resize(10, 0);

  for (const auto& p : x) {
    std::printf("%d\n", p);
  }
  std::printf("--\n");
  x.resize(5);
  for (const auto& p : x) {
    std::printf("%d\n", p);
  }
}
