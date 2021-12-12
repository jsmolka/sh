#pragma once

#include <memory>
#include <ostream>
#include <string>

struct type1 {
  type1() : type1(0) {}
  type1(int value) : value(value) {}
  type1(type1&&) = default;
  type1(const type1&) = default;

  auto operator=(type1&&) -> type1& = default;
  auto operator=(const type1&) -> type1& = default;
  auto operator<=>(const type1&) const = default;

  int value;
};

struct type2 {
  type2() : type2(0) {}
  type2(int value) : value(std::to_string(value)) {}
  type2(type2&&) = default;
  type2(const type2&) = default;

  auto operator=(type2&&) -> type2& = default;
  auto operator=(const type2&) -> type2& = default;
  auto operator<=>(const type2&) const = default;

  std::string value;
};

struct type3 {
  type3() : type3(0) {}
  type3(int value) : value(std::to_string(value)) {}
  type3(type3&&) = delete;
  type3(const type3&) = default;

  auto operator=(type3&&) -> type3& = default;
  auto operator=(const type3&) -> type3& = default;
  auto operator<=>(const type3&) const = default;

  std::string value;
};

struct type4 {
  type4() : type4(0) {}
  type4(int value) : value(std::make_unique<int>(value)) {}
  type4(type4&& other) noexcept : value(std::move(other.value)) {}
  type4(const type4& other) : type4(*other.value) {}

  auto operator=(type4&& other) noexcept -> type4& {
    if (this != &other) [[likely]] {
      value = std::move(other.value);
    }
    return *this;
  }

  auto operator=(const type4& other) -> type4& {
    if (this != &other) [[likely]] {
      value = std::make_unique<int>(*other.value);
    }
    return *this;
  }

  auto operator==(const type4& other) const -> bool {
    return *value == *other.value;
  }

  std::unique_ptr<int> value;
};

template <sh::any_of<type1, type2, type3, type4> T>
auto operator<<(std::ostream& out, const T& value) -> std::ostream& {
  return out << value.value;
}
