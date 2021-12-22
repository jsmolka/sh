#pragma once

#include <sh/hash.h>

#include "ut.h"

namespace tests_hash {

inline suite _ = [] {
  "hash value"_test = [] {
    expect(eq(sh::hash(0x1234'5678), 0x6A29'5429'B2D6'B891));
  };

  "hash range"_test = [] {
    int data[] = {0x1234'5678, 0x1234'5678, 0x1234'5678, 0x1234'5678};
    expect(eq(sh::hash(std::begin(data), std::end(data)), 0xAFFC'E2EE'423B'D28D));
  };
};

}  // namespace tests_hash
