#include "tests_hash.h"

#include "common.h"

void tests_hash() {
  sh::test("hash value") = []() {
    expect(eq(sh::hash(0x1234'5678), 0x6A29'5429'B2D6'B891));
  };

  sh::test("hash range") = []() {
    int data[] = {0x1234'5678, 0x1234'5678, 0x1234'5678, 0x1234'5678};
    expect(eq(sh::hash(std::begin(data), std::end(data)), 0xAFFC'E2EE'423B'D28D));
  };
}
