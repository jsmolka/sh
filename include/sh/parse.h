#pragma once

#include <charconv>
#include <optional>
#include <string_view>

#include <sh/int.h>

namespace sh {

template <typename T>
auto parse(std::string_view data) -> std::optional<T> {
  return T{};
}

template <std::integral T>
auto parse(std::string_view data) -> std::optional<T> {
  T value{};
  const auto& [ptr, ec] =
      std::from_chars(std::to_address(data.begin()), std::to_address(data.end()), value);
  if (ec == std::errc::result_out_of_range || ptr != std::to_address(data.end())) {
    return std::nullopt;
  }
  return value;
}

}  // namespace sh
