#pragma once

#include <charconv>
#include <concepts>
#include <optional>
#include <string_view>

namespace sh {

template <typename T>
auto parse(std::string_view data) -> std::optional<T> {
  return T{};
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

  Integral value{};
  const auto beg = data.data();
  const auto end = data.data() + data.size();
  const auto& [ptr, ec] = std::from_chars(beg, end, value, base);

  if (base != 10 && negative) {
    std::swap(*sign, *temp);
  }

  if (ec == std::errc::result_out_of_range || ptr != end) {
    return std::nullopt;
  }
  return value;
}

}  // namespace sh
