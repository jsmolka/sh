#include "tests_hash.h"

#include <sh/hash.h>

#include "ut.h"

void tests_hash() {
  u64 seed = sh::hash(0x1234'5678);
  expect(eq(seed, 0x6A29'5429'B2D6'B891));

  int data[] = {0x1234'5678, 0x1234'5678, 0x1234'5678, 0x1234'5678};
  seed = sh::hash(std::begin(data), std::end(data));
  expect(eq(seed, 0xAFFC'E2EE'423B'D28D));
}
