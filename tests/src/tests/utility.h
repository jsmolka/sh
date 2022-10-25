#pragma once

#include <sh/int.h>
#include <sh/utility.h>

#include "ut.h"

namespace tests_utility {

using sh::u8;
using sh::u16;
using sh::u32;
using sh::u64;

 SH_INLINE void test_unreachable(int x) {
  switch (x) {
    case 0:
      expect(true);
      break;
    default:
      SH_UNREACHABLE;
      break;
  }
}

 SH_NO_INLINE void test_macros() {
  test_unreachable(0);
}

inline suite _ = [] {
  "macros"_test = [] {
    test_macros();
  };

  "unused"_test = [] {
    int value = 0;
    sh::unused(value);
  };

  "cast<T&>"_test = [] {
    u32 data = 0;
    sh::cast<u8>(data, 0) = 0x01;
    sh::cast<u8>(data, 1) = 0x02;
    sh::cast<u8>(data, 2) = 0x03;
    sh::cast<u8>(data, 3) = 0x04;
    expect(eq(data, 0x04030201));
  };

  "cast<const T&>"_test = [] {
    const u32 data = 0x04030201;
    expect(eq(sh::cast<u16>(data, 0), 0x0201));
    expect(eq(sh::cast<u16>(data, 2), 0x0403));
  };

  "cast<T>"_test = [] {
    expect(eq(sh::cast<u16>(0x04030201, 0), 0x0201));
    expect(eq(sh::cast<u16>(0x04030201, 2), 0x0403));
  };

  "reconstruct"_test = [] {
    struct reconstruct {
      reconstruct() = default;
      reconstruct(int v) : v(v) {}
      int v = 0;
    };

    auto test = [&]() SH_INLINE_LAMBDA {
      expect(true);
    };
    test();

    reconstruct r;
    expect(eq(r.v, 0));
    sh::reconstruct(r, 1);
    expect(eq(r.v, 1));
    sh::reconstruct(r);
    expect(eq(r.v, 0));
  };
};

}  // namespace tests_utility

