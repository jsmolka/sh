#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string>
#include <string_view>

#include <sh/concepts.h>
#include <sh/fast_float/fast_float.h>

namespace std {

using fast_float::from_chars;

}  // namespace std

namespace sh {

namespace {

template <typename T, typename... Args>
auto parse_number(std::string_view data, Args&&... args) -> std::optional<T> {
  T value{};
  const auto beg = data.data();
  const auto end = data.data() + data.size();
  const auto& [ptr, ec] = std::from_chars(beg, end, value, std::forward<Args>(args)...);
  if (ec == std::errc::result_out_of_range || ptr != end) {
    return std::nullopt;
  }
  return value;
}

}  // namespace

template <typename T>
auto parse(std::string_view data) -> std::optional<T> {
  return std::nullopt;
}

template <std::integral Integral>
auto parse(std::string_view data) -> std::optional<Integral> {
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

  const auto value = parse_number<Integral>(data, base);
  if (base != 10 && negative) {
    std::swap(*sign, *temp);
  }
  return value;
}

template <>
inline auto parse(std::string_view data) -> std::optional<bool> {
  if (data == "1" || data == "true") {
    return true;
  } else if (data == "0" || data == "false") {
    return false;
  } else {
    return std::nullopt;
  }
}

template <sh::any_of<float, double> Float>
auto parse(std::string_view data) -> std::optional<Float> {
  return parse_number<Float>(data);
}

template <sh::any_of<std::string, std::string_view> String>
auto parse(std::string_view data) -> std::optional<String> {
  return String(data.begin(), data.end());
}

template <typename T>
concept parsable = requires() {
  { parse<T>(std::string_view{}) } -> std::same_as<std::optional<T>>;
};

}  // namespace sh
