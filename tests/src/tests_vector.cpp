#include "tests_vector.h"

#include <compare>
#include <vector>

#include <sh/vector.h>

#include "ut.h"

struct trivially_copyable {
  trivially_copyable() : trivially_copyable(0) {}
  trivially_copyable(int value) : value(value) {}
  trivially_copyable(const trivially_copyable&) noexcept = default;
  trivially_copyable(trivially_copyable&&) noexcept = default;
  trivially_copyable& operator=(trivially_copyable&&) noexcept = default;
  trivially_copyable& operator=(const trivially_copyable&) noexcept = default;
  auto operator<=>(const trivially_copyable&) const = default;
  int value;
};

static_assert(std::is_trivially_copyable_v<trivially_copyable>);
static_assert(std::is_nothrow_move_constructible_v<trivially_copyable>);
static_assert(std::is_move_constructible_v<trivially_copyable>);
static_assert(std::is_nothrow_copy_constructible_v<trivially_copyable>);
static_assert(std::is_copy_constructible_v<trivially_copyable>);

struct nothrow_move_constructible {
  nothrow_move_constructible() : nothrow_move_constructible(0) {}
  nothrow_move_constructible(int value) : value(std::to_string(value)) {}
  nothrow_move_constructible(nothrow_move_constructible&&) noexcept = default;
  nothrow_move_constructible(const nothrow_move_constructible&) noexcept = default;
  nothrow_move_constructible& operator=(nothrow_move_constructible&&) noexcept = default;
  nothrow_move_constructible& operator=(const nothrow_move_constructible&) noexcept = default;
  auto operator<=>(const nothrow_move_constructible&) const = default;
  std::string value;
};

static_assert(!std::is_trivially_copyable_v<nothrow_move_constructible>);
static_assert(std::is_nothrow_move_constructible_v<nothrow_move_constructible>);
static_assert(std::is_move_constructible_v<nothrow_move_constructible>);
static_assert(std::is_nothrow_copy_constructible_v<nothrow_move_constructible>);
static_assert(std::is_copy_constructible_v<nothrow_move_constructible>);

struct move_constructible {
  move_constructible() : move_constructible(0) {}
  move_constructible(int value) : value(std::to_string(value)) {}
  move_constructible(move_constructible&&) noexcept(false) = default;
  move_constructible(const move_constructible&) noexcept = default;
  move_constructible& operator=(move_constructible&&) noexcept = default;
  move_constructible& operator=(const move_constructible&) noexcept = default;
  auto operator<=>(const move_constructible&) const = default;
  std::string value;
};

static_assert(!std::is_trivially_copyable_v<move_constructible>);
static_assert(!std::is_nothrow_move_constructible_v<move_constructible>);
static_assert(std::is_move_constructible_v<move_constructible>);
static_assert(std::is_nothrow_copy_constructible_v<move_constructible>);
static_assert(std::is_copy_constructible_v<move_constructible>);

struct nothrow_copy_constructible {
  nothrow_copy_constructible() : nothrow_copy_constructible(0) {}
  nothrow_copy_constructible(int value) : value(std::to_string(value)) {}
  nothrow_copy_constructible(nothrow_copy_constructible&&) noexcept(false) = delete;
  nothrow_copy_constructible(const nothrow_copy_constructible&) noexcept = default;
  nothrow_copy_constructible& operator=(nothrow_copy_constructible&&) = default;
  nothrow_copy_constructible& operator=(const nothrow_copy_constructible&) = default;
  auto operator<=>(const nothrow_copy_constructible&) const = default;
  std::string value;
};

static_assert(!std::is_trivially_copyable_v<nothrow_copy_constructible>);
static_assert(!std::is_nothrow_move_constructible_v<nothrow_copy_constructible>);
static_assert(!std::is_move_constructible_v<nothrow_copy_constructible>);
static_assert(std::is_nothrow_copy_constructible_v<nothrow_copy_constructible>);
static_assert(std::is_copy_constructible_v<nothrow_copy_constructible>);

struct copy_constructible {
  copy_constructible() : copy_constructible(0) {}
  copy_constructible(int value) : value(std::to_string(value)) {}
  copy_constructible(copy_constructible&&) = delete;
  copy_constructible(const copy_constructible&) noexcept(false) = default;
  copy_constructible& operator=(copy_constructible&&) = default;
  copy_constructible& operator=(const copy_constructible&) = default;
  auto operator<=>(const copy_constructible&) const = default;
  std::string value;
};

static_assert(!std::is_trivially_copyable_v<copy_constructible>);
static_assert(!std::is_nothrow_move_constructible_v<copy_constructible>);
static_assert(!std::is_move_constructible_v<copy_constructible>);
static_assert(!std::is_nothrow_copy_constructible_v<copy_constructible>);
static_assert(std::is_copy_constructible_v<copy_constructible>);

template <typename T>
void test_vector_constructor() {
  make_test("vector<{}>::vector()", typeid(T).name()) = []() {
    sh::vector<T> v1;
    expect(v1.size() == 0);
    expect(v1.capacity() == 0);
    expect(v1.data() == nullptr);
    expect(v1.begin() == nullptr);
    expect(v1.end() == nullptr);
  };

  make_test("vector<{}>::vector(size_type)", typeid(T).name()) = []() {
    constexpr auto kCount = 3;
    sh::vector<T> v1(kCount);
    expect(v1.size() == kCount);
    expect(v1.capacity() == kCount);

    int i = 0;
    for (const auto& x : v1) {
      expect(x == T{});
      i++;
    }
    expect(i == kCount);
  };

  make_test("vector<{}>::vector(size_type, const value_type&)", typeid(T).name()) = []() {
    constexpr auto kCount = 3;
    sh::vector<T> v1(kCount, {1});
    expect(v1.size() == kCount);
    expect(v1.capacity() == kCount);

    int i = 0;
    for (const auto& x : v1) {
      expect(x == T{1});
      i++;
    }
    expect(i == kCount);
  };

  make_test("vector<{}>::vector(const_iterator, const_iterator)", typeid(T).name()) = []() {
    sh::vector<T> v1;
    v1.emplace_back(0);
    v1.emplace_back(1);
    v1.emplace_back(2);

    sh::vector<T> v2(v1.begin(), v1.end());
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(v2[0] == v1[0]);
    expect(v2[1] == v1[1]);
    expect(v2[2] == v2[2]);
  };

  make_test("vector<{}>::vector(initializer_list)", typeid(T).name()) = []() {
    sh::vector<T> v1 = {0, 1, 2};
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(v1[0] == T{0});
    expect(v1[1] == T{1});
    expect(v1[2] == T{2});

    sh::vector<T> v2 = {};
    expect(v2.size() == 0);
    expect(v2.capacity() == 0);
    expect(v2.data() == nullptr);
    expect(v2.begin() == nullptr);
    expect(v2.end() == nullptr);
  };

  make_test("vector<{}>::vector(const vector&)", typeid(T).name()) = []() {
    sh::vector<T> v1 = {0, 1, 2};
    sh::vector<T> v2(v1);
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(v1[0] == v2[0]);
    expect(v1[1] == v2[1]);
    expect(v1[2] == v2[2]);
  };

  make_test("vector<{}>::vector(vector&&)", typeid(T).name()) = []() {
    sh::vector<T> v1 = {0, 1, 2};
    const auto data = v1.data();
    sh::vector<T> v2(std::move(v1));
    expect(v1.data() == nullptr);
    expect(v2.data() == data);
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(v2[0] == T{0});
    expect(v2[1] == T{1});
    expect(v2[2] == T{2});
  };
}

template <typename T>
void test_vector_erase() {
  const auto name = make_test("vector<{}>::erase(const_iterator)", typeid(T).name());
  test(name) = []() {
    sh::vector<T> v;
    v.emplace_back(0);
    v.emplace_back(1);
    v.emplace_back(2);
    expect(v.size() == 3);

    auto i = v.erase(v.begin() + 1);
    expect(v.size() == 2);
    expect(*i == T{2});
  };
}

void tests_vector() {
  test_vector_constructor<trivially_copyable>();
  test_vector_constructor<nothrow_move_constructible>();
  test_vector_constructor<move_constructible>();
  test_vector_constructor<nothrow_copy_constructible>();
  test_vector_constructor<copy_constructible>();
  // test_vector_erase<trivially_copyable>();
  // test_vector_erase<nothrow_move_constructible>();
  // test_vector_erase<move_constructible>();
  // test_vector_erase<nothrow_copy_constructible>();
  // test_vector_erase<copy_constructible>();
}
