#include "tests_utility.h"

#include "common.h"

namespace {

SH_NO_INLINE void test_unreachable(int x) {
  switch (x) {
    case 0:
      expect(true);
      break;
    default:
      SH_UNREACHABLE;
      break;
  }
}

SH_INLINE void test_macros() {
  test_unreachable(0);
}

}  // namespace

void tests_utility() {
  sh::test("macros") = []() {
    test_macros();
  };

  sh::test("unused") = []() {
    int value = 0;
    sh::unused(value);
  };

  sh::test("cast<T&>") = []() {
    u32 data = 0;
    sh::cast<u8>(data, 0) = 0x01;
    sh::cast<u8>(data, 1) = 0x02;
    sh::cast<u8>(data, 2) = 0x03;
    sh::cast<u8>(data, 3) = 0x04;
    expect(eq(data, 0x04030201));
  };

  sh::test("cast<const T&>") = []() {
    const u32 data = 0x04030201;
    expect(eq(sh::cast<u16>(data, 0), 0x0201));
    expect(eq(sh::cast<u16>(data, 2), 0x0403));
  };

  sh::test("cast<T>") = []() {
    expect(eq(sh::cast<u16>(0x04030201, 0), 0x0201));
    expect(eq(sh::cast<u16>(0x04030201, 2), 0x0403));
  };
}
