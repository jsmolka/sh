#include "tests_parse.h"

#include <numeric>

#include <sh/parse.h>

#include "ut.h"

namespace {

template <typename T>
struct tests_integral {
  static auto test(std::string_view what) {
    return ::test("parse<{}>::{}", typeid(T).name(), what);
  }

  template <typename Format>
  static void linear(Format format) {
    using counter = std::conditional_t<std::is_signed_v<T>, s64, u64>;

    const auto step = [](u64 value) -> u64 {
      if (value <= std::numeric_limits<u8>::max()) {
        return std::numeric_limits<u8>::max() >> 4;
      } else if (value <= std::numeric_limits<u16>::max()) {
        return std::numeric_limits<u16>::max() >> 4;
      } else if (value <= std::numeric_limits<u32>::max()) {
        return std::numeric_limits<u32>::max() >> 4;
      } else {
        return std::numeric_limits<u64>::max() >> 4;
      }
    };

    auto expected = std::numeric_limits<counter>::min();
    for (; expected + step(expected) > expected; expected += step(expected)) {
      const auto value = sh::parse<T>(format(expected));
      if (expected >= std::numeric_limits<T>::min() && expected <= std::numeric_limits<T>::max()) {
        expect(value.has_value());
        expect(eq(static_cast<counter>(*value), expected));
      } else {
        expect(!value.has_value());
      }
    }
  }

  static void run() {
    test("decimal") = []() {
      linear([](auto value) {
        return fmt::format("{}", value);
      });
    };

    test("binary") = []() {
      linear([](auto value) {
        return fmt::format("{:#b}", value);
      });
    };

    test("hexadecimal") = []() {
      linear([](auto value) {
        return fmt::format("{:#x}", value);
      });
    };
  }
};  // namespace

}  // namespace

void tests_parse() {
  tests_integral<u8>::run();
  tests_integral<u16>::run();
  tests_integral<u32>::run();
  tests_integral<u64>::run();
  tests_integral<s8>::run();
  tests_integral<s16>::run();
  tests_integral<s32>::run();
  tests_integral<s64>::run();
}
