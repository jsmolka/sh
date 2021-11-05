#include "tests_vector.h"

#include <sh/vector.h>

#include "ut.h"

struct trivially_copyable {
  int x;
};

static_assert(std::is_trivially_copyable_v<trivially_copyable>);
static_assert(std::is_nothrow_move_constructible_v<trivially_copyable>);
static_assert(std::is_move_constructible_v<trivially_copyable>);
static_assert(std::is_nothrow_copy_constructible_v<trivially_copyable>);
static_assert(std::is_copy_constructible_v<trivially_copyable>);

struct nothrow_move_constructible {
  nothrow_move_constructible(int x) : x(std::to_string(x)) {}
  nothrow_move_constructible(nothrow_move_constructible&&) = default;

  std::string x;
};

static_assert(!std::is_trivially_copyable_v<nothrow_move_constructible>);
static_assert(std::is_nothrow_move_constructible_v<nothrow_move_constructible>);
static_assert(std::is_move_constructible_v<nothrow_move_constructible>);
static_assert(!std::is_nothrow_copy_constructible_v<nothrow_move_constructible>);
static_assert(!std::is_copy_constructible_v<nothrow_move_constructible>);

struct move_constructible {
  move_constructible(int x) : x(std::to_string(x)) {}
  move_constructible(move_constructible&&) noexcept(false) = default;

  std::string x;
};

static_assert(!std::is_trivially_copyable_v<move_constructible>);
static_assert(!std::is_nothrow_move_constructible_v<move_constructible>);
static_assert(std::is_move_constructible_v<move_constructible>);
static_assert(!std::is_nothrow_copy_constructible_v<move_constructible>);
static_assert(!std::is_copy_constructible_v<move_constructible>);

struct nothrow_copy_constructible {
  nothrow_copy_constructible(int x) : x(std::to_string(x)) {}
  nothrow_copy_constructible(nothrow_copy_constructible&&) = delete;
  nothrow_copy_constructible(const nothrow_copy_constructible&) noexcept = default;

  std::string x;
};

static_assert(!std::is_trivially_copyable_v<nothrow_copy_constructible>);
static_assert(!std::is_nothrow_move_constructible_v<nothrow_copy_constructible>);
static_assert(!std::is_move_constructible_v<nothrow_copy_constructible>);
static_assert(std::is_nothrow_copy_constructible_v<nothrow_copy_constructible>);
static_assert(std::is_copy_constructible_v<nothrow_copy_constructible>);

struct copy_constructible {
  copy_constructible(int x) : x(std::to_string(x)) {}
  copy_constructible(copy_constructible&&) = delete;
  copy_constructible(const copy_constructible&) noexcept(false) = default;

  std::string x;
};

template <typename T>
void test_vector_erase() {
  const auto name = make_name("vector<", typeid(T).name(), ">::erase");
  test(name) = []() {
    expect(1 == 2);
  };
}

void tests_vector() {
  test_vector_erase<trivially_copyable>();
  test_vector_erase<nothrow_move_constructible>();
  test_vector_erase<move_constructible>();
  test_vector_erase<nothrow_copy_constructible>();
  test_vector_erase<copy_constructible>();
}
