#pragma once

#include <numeric>

#include <sh/int.h>
#include <sh/parse.h>

#include "ut.h"

namespace tests_parse {

using sh::u8;
using sh::u16;
using sh::u32;
using sh::u64;
using sh::s8;
using sh::s16;
using sh::s32;
using sh::s64;

template<typename T>
struct tests {
  static auto test(std::string_view what) {
    return testf("parse<{}> {}", typeid(T).name(), what);
  }
};

template<typename T>
struct tests_integer : tests<T> {
  using tests<T>::test;

  template<typename String, typename Format>
  static void linear(Format format) {
    using counter = std::conditional_t<std::is_signed_v<T>, s64, u64>;

    auto step = [](u64 value) -> u64 {
      return [&]() -> u64 {
        if (value <= std::numeric_limits<u8>::max()) {
          return std::numeric_limits<u8>::max();
        } else if (value <= std::numeric_limits<u16>::max()) {
          return std::numeric_limits<u16>::max();
        } else if (value <= std::numeric_limits<u32>::max()) {
          return std::numeric_limits<u32>::max();
        } else {
          return std::numeric_limits<u64>::max();
        }
      }() >> 3;
    };

    auto expected = std::numeric_limits<counter>::min();
    for (; expected + step(expected) > expected; expected += step(expected)) {
      const auto string = format(expected);
      const auto value = sh::parse<T>(static_cast<String>(string));
      if (expected >= std::numeric_limits<T>::min() && expected <= std::numeric_limits<T>::max()) {
        expect(value.has_value());
        expect(eq(static_cast<counter>(*value), expected));
      } else {
        expect(!value.has_value());
      }
    }
  }

  static void run() {
    test("decimal") = [] {
      const auto format = [](auto value) {
        return fmt::format("{}", value);
      };
      linear<std::string>(format);
      linear<std::string_view>(format);
    };

    test("binary") = [] {
      const auto format = [](auto value) {
        return fmt::format("{:#b}", value);
      };
      linear<std::string>(format);
      linear<std::string_view>(format);
    };

    test("hexadecimal") = [] {
      const auto format = [](auto value) {
        return fmt::format("{:#x}", value);
      };
      linear<std::string>(format);
      linear<std::string_view>(format);
    };
  }
};

template<typename T>
struct tests_float : tests<T> {
  using tests<T>::test;

  template<typename String>
  static void linear() {
    for (auto expected = static_cast<T>(-100.0); expected < 100.0; expected += 10.1232456789) {
      const auto string = fmt::format("{:f}", expected);
      const auto value = sh::parse<T>(static_cast<String>(string));
      expect(value.has_value());
      expect(eq(_d(*value, 1e-6), _d(expected)));
    }
  }

  static void run() {
    test("decimal") = [] {
      linear<std::string>();
      linear<std::string_view>();
    };
  }
};

inline suite _ = [] {
  tests_integer<sh::u8>::run();
  tests_integer<sh::u16>::run();
  tests_integer<sh::u32>::run();
  tests_integer<sh::u64>::run();
  tests_integer<sh::s8>::run();
  tests_integer<sh::s16>::run();
  tests_integer<sh::s32>::run();
  tests_integer<sh::s64>::run();
  tests_float<float>::run();
  tests_float<double>::run();

  "parse<bool>"_test = [] {
    expect(eq(*sh::parse<bool>("true"), true));
    expect(eq(*sh::parse<bool>("1"), true));
    expect(eq(*sh::parse<bool>("false"), false));
    expect(eq(*sh::parse<bool>("0"), false));
    expect(!sh::parse<bool>(" "));
  };
};

}  // namespace tests_parse
