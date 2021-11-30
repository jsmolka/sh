#include "tests_pythonic.h"

#include <list>
#include <vector>

#include <sh/pythonic.h>

#include "ut.h"

void tests_pythonic() {
  std::vector<int> x = {1, 2, 3};

  for (const auto& [index, value] : sh::enumerate(x)) {
    int y = 0;
  }
}
