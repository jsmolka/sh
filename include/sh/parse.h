#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>

#include <sh/deps/fast_float/include/fast_float/fast_float.h>
#include <sh/int.h>

namespace std {

using fast_float::from_chars;

}  // namespace std

namespace sh {

namespace detail {

template<typename T>
struct number_parser {
  template<typename... Args>
  auto parse(std::string_view data, Args&&... args) -> std::optional<T> {
    T value{};
    const auto beg = data.data();
    const auto end = data.data() + data.size();
    const auto& [ptr, ec] = std::from_chars(beg, end, value, std::forward<Args>(args)...);
    if (ec == std::errc::result_out_of_range || ptr != end) {
      return std::nullopt;
    }
    return value;
  }
};

template<typename T>
struct int_parser : number_parser<T> {
  auto parse(std::string_view data) -> std::optional<T> {
    const auto negative = data.starts_with('-');
    const auto base = [&]() -> int {
      const auto index = std::size_t(negative);
      if (index + 1 < data.size() && data[index] == '0') {
        switch (data[index + 1]) {
          case 'b':
          case 'B':
            return 2;
          case 'x':
          case 'X':
            return 16;
        }
      }
      return 10;
    }();

    auto sign = const_cast<char*>(data.data());
    auto temp = const_cast<char*>(data.data()) + 2;
    if (base != 10) {
      data.remove_prefix(2);
      if (negative) {
        std::swap(*sign, *temp);
      }
    }

    const auto value = number_parser<T>::parse(data, base);
    if (base != 10 && negative) {
      std::swap(*sign, *temp);
    }
    return value;
  }
};

}  // namespace detail

template<typename T>
struct parser {
  ~parser() = delete;
};

template<> struct parser<u8>  : detail::int_parser<u8>  {}; 
template<> struct parser<u16> : detail::int_parser<u16> {};
template<> struct parser<u32> : detail::int_parser<u32> {};
template<> struct parser<u64> : detail::int_parser<u64> {};
template<> struct parser<s8>  : detail::int_parser<s8>  {}; 
template<> struct parser<s16> : detail::int_parser<s16> {};
template<> struct parser<s32> : detail::int_parser<s32> {};
template<> struct parser<s64> : detail::int_parser<s64> {};

template<>
struct parser<bool> {
  auto parse(std::string_view data) -> std::optional<bool> {
    if (data == "1" || data == "true") {
      return true;
    } else if (data == "0" || data == "false") {
      return false;
    } else {
      return std::nullopt;
    }
  }
};

template<> struct parser<float>  : detail::number_parser<float>  {};
template<> struct parser<double> : detail::number_parser<double> {};

template<>
struct parser<std::string> {
  auto parse(std::string_view data) -> std::optional<std::string> {
    return std::string(data);
  }
};

template<>
struct parser<std::string_view> {
  auto parse(std::string_view data) -> std::optional<std::string_view> {
    return data;
  }
};

template<typename T>
concept parsable = std::destructible<parser<T>>;

template<parsable T>
auto parse(std::string_view data) -> std::optional<T> {
  return parser<T>{}.parse(data);
}

}  // namespace sh
