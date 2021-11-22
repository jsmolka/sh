#pragma once

#include <compare>
#include <memory>
#include <ostream>
#include <string>

struct test_type1 {
  test_type1() : test_type1(0) {}
  test_type1(int value) : value(value) {}
  test_type1(test_type1&&) = default;
  test_type1(const test_type1&) = default;

  auto operator=(test_type1&&) -> test_type1& = default;
  auto operator=(const test_type1&) -> test_type1& = default;
  auto operator<=>(const test_type1&) const = default;

  int value;
};

struct test_type2 {
  test_type2() : test_type2(0) {}
  test_type2(int value) : value(std::to_string(value)) {}
  test_type2(test_type2&&) = default;
  test_type2(const test_type2&) = default;

  auto operator=(test_type2&&) -> test_type2& = default;
  auto operator=(const test_type2&) -> test_type2& = default;
  auto operator<=>(const test_type2&) const = default;

  std::string value;
};

struct test_type3 {
  test_type3() : test_type3(0) {}
  test_type3(int value) : value(std::to_string(value)) {}
  test_type3(test_type3&&) = delete;
  test_type3(const test_type3&) = default;

  auto operator=(test_type3&&) -> test_type3& = default;
  auto operator=(const test_type3&) -> test_type3& = default;
  auto operator<=>(const test_type3&) const = default;

  std::string value;
};

struct test_type4 {
  test_type4() : test_type4(0) {}
  test_type4(int value) : value(std::make_unique<int>(value)) {}
  test_type4(test_type4&& other) noexcept : value(std::move(other.value)) {}
  test_type4(const test_type4& other) : test_type4(*other.value) {}

  auto operator=(test_type4&& other) noexcept -> test_type4& {
    if (this != &other) [[likely]] {
      value = std::move(other.value);
    }
    return *this;
  }

  auto operator=(const test_type4& other) -> test_type4& {
    if (this != &other) [[likely]] {
      value = std::make_unique<int>(*other.value);
    }
    return *this;
  }

  auto operator==(const test_type4& other) const -> bool {
    return *value == *other.value;
  }

  std::unique_ptr<int> value;
};

template <typename T>
concept test_type = std::constructible_from<T, int> && requires(T&& t) {
  t.value;
};

template <test_type T>
auto operator<<(std::ostream& out, const T& value) -> std::ostream& {
  out << value.value;
  return out;
}
