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
auto make_vector_test(std::string_view function) {
  return make_test("vector<{}>::{}", typeid(T).name(), function);
}

template <typename T>
void tests_vector_constructor() {
  make_vector_test<T>("vector()") = []() {
    sh::vector<T> v1;
    expect(v1.size() == 0);
    expect(v1.capacity() == 0);
    expect(v1.data() == nullptr);
    expect(v1.begin() == nullptr);
    expect(v1.end() == nullptr);
    expect(static_cast<bool>(v1 == sh::vector<T>{}));
  };

  make_vector_test<T>("vector(size_type, const value_type&)") = []() {
    sh::vector<T> v1(3, {1});
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{1, 1, 1}));

    sh::vector<T> v2(0, {1});
    expect(v2.size() == 0);
    expect(v2.capacity() == 0);
    expect(static_cast<bool>(v2 == sh::vector<T>{}));
  };

  make_vector_test<T>("vector(size_type)") = []() {
    sh::vector<T> v1(3);
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{0, 0, 0}));

    sh::vector<T> v2(0);
    expect(v2.size() == 0);
    expect(v2.capacity() == 0);
    expect(static_cast<bool>(v2 == sh::vector<T>{}));
  };

  make_vector_test<T>("vector(InputIt, InputIt)") = []() {
    sh::vector<T> v1{0, 1, 2};
    sh::vector<T> v2(v1.begin(), v1.end());
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v1 == v2));

    sh::vector<T> v3;
    sh::vector<T> v4(v3.begin(), v3.end());
    expect(v3.size() == 0);
    expect(v3.capacity() == 0);
    expect(static_cast<bool>(v3 == v4));
  };

  make_vector_test<T>("vector(initializer_list)") = []() {
    sh::vector<T> v1{0, 1, 2};
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(v1[0] == T{0});
    expect(v1[1] == T{1});
    expect(v1[2] == T{2});
  };

  make_vector_test<T>("vector(const vector&)") = []() {
    sh::vector<T> v1{0, 1, 2};
    sh::vector<T> v2(v1);
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v1 == v2));
  };

  make_vector_test<T>("vector(vector&&)") = []() {
    sh::vector<T> v1{0, 1, 2};
    const auto data = v1.data();
    sh::vector<T> v2(std::move(v1));
    expect(v1.data() == nullptr);
    expect(v2.data() == data);
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v2 == sh::vector<T>{0, 1, 2}));
  };
}

template <typename T>
void tests_vector_comparison_operator() {
  make_vector_test<T>("operator==") = []() {
    sh::vector<T> v1{0, 1, 2};
    sh::vector<T> v2{0, 1, 2};
    expect(static_cast<bool>(v1 == v2));
  };

  make_vector_test<T>("operator!=") = []() {
    sh::vector<T> v1{0, 1, 2};
    sh::vector<T> v2{0, 1};
    expect(static_cast<bool>(v1 != v2));

    sh::vector<T> v3{0, 1, 2};
    sh::vector<T> v4{0, 1, 3};
    expect(static_cast<bool>(v3 != v4));
  };
}

template <typename T>
void tests_vector_assign() {
  make_vector_test<T>("assign(size_type, const value_type&)") = []() {
    sh::vector<T> v1;
    v1.assign(3, {1});
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{1, 1, 1}));

    v1.assign(0, {2});
    expect(v1.size() == 0);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{}));
  };

  make_vector_test<T>("assign(InputIt, InputIt)") = []() {
    sh::vector<T> v1{0, 1, 2};
    sh::vector<T> v2;
    v2.assign(v1.begin(), v1.end());
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{0, 1, 2}));

    sh::vector<T> v3;
    v2.assign(v3.begin(), v3.end());
    expect(v2.size() == 0);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v2 == sh::vector<T>{}));
  };

  make_vector_test<T>("assign(initializer_list)") = []() {
    sh::vector<T> v1;
    v1.assign({0, 1, 2});
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{0, 1, 2}));

    v1.assign({});
    expect(v1.size() == 0);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{}));
  };
}

template <typename T>
void tests_vector_assignment_operator() {
  make_vector_test<T>("operator=(const vector&)") = []() {
    sh::vector<T> v1{0, 1, 2};
    sh::vector<T> v2 = v1;
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v1 == v2));

    sh::vector<T> v3;
    sh::vector<T> v4 = v3;
    expect(v4.size() == 0);
    expect(v4.capacity() == 0);
    expect(static_cast<bool>(v3 == v4));
  };

  make_vector_test<T>("operator=(vector&&)") = []() {
    sh::vector<T> v1{0, 1, 2};
    const auto data = v1.data();
    sh::vector<T> v2 = std::move(v1);
    expect(v1.data() == nullptr);
    expect(v2.data() == data);
    expect(v2.size() == 3);
    expect(v2.capacity() == 3);
    expect(static_cast<bool>(v2 == sh::vector<T>{0, 1, 2}));
  };

  make_vector_test<T>("operator=(initializer_list)") = []() {
    sh::vector<T> v1 = {0, 1, 2};
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{0, 1, 2}));

    v1 = {};
    expect(v1.size() == 0);
    expect(v1.capacity() == 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{}));
  };
}

template <typename T>
void tests_vector_reserve() {
  make_vector_test<T>("reserve(size_type)") = []() {
    sh::vector<T> v1;
    v1.reserve(10);
    expect(v1.capacity() == 10);
    v1.reserve(20);
    expect(v1.capacity() == 20);
    v1.reserve(10);
    expect(v1.capacity() == 20);
    expect(v1.size() == 0);
  };
}

template <typename T>
void tests_vector_shrink_to_fit() {
  make_vector_test<T>("shrink_to_fit()") = []() {
    sh::vector<T> v1;
    v1.reserve(10);
    v1.assign(5, {0});
    expect(v1.size() == 5);
    expect(v1.capacity() == 10);
    v1.shrink_to_fit();
    expect(v1.size() == 5);
    expect(v1.capacity() == 5);

    sh::vector<T> v2;
    v2.shrink_to_fit();
    expect(v2.size() == 0);
    expect(v2.capacity() == 0);
  };
}

template <typename T>
void tests_vector_clear() {
  make_vector_test<T>("clear()") = []() {
    sh::vector<T> v1{0, 1, 2};
    expect(v1.size() == 3);
    expect(v1.capacity() == 3);
    v1.clear();
    expect(v1.size() == 0);
    expect(v1.capacity() == 3);
  };
}

template <typename T>
void tests_vector_emplace() {
  make_vector_test<T>("emplace(const_iterator, Args...)") = []() {
    sh::vector<T> v1{0, 0};
    v1.emplace(v1.begin(), 1);
    v1.emplace(v1.begin() + 2, 2);
    v1.emplace(v1.end(), 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{1, 0, 2, 0, 3}));
  };
}

template <typename T>
void tests_vector_insert() {
  make_vector_test<T>("insert(const_iterator, const value_type&|value_type&&)") = []() {
    sh::vector<T> v1{0, 0};
    v1.insert(v1.begin(), 1);
    v1.insert(v1.begin() + 2, 2);
    v1.insert(v1.end(), 3);
    expect(static_cast<bool>(v1 == sh::vector<T>{1, 0, 2, 0, 3}));
  };
}

void tests_vector() {
  tests_vector_constructor<trivially_copyable>();
  tests_vector_constructor<nothrow_move_constructible>();
  tests_vector_constructor<move_constructible>();
  tests_vector_constructor<nothrow_copy_constructible>();
  tests_vector_constructor<copy_constructible>();
  tests_vector_comparison_operator<trivially_copyable>();
  tests_vector_comparison_operator<nothrow_move_constructible>();
  tests_vector_comparison_operator<move_constructible>();
  tests_vector_comparison_operator<nothrow_copy_constructible>();
  tests_vector_comparison_operator<copy_constructible>();
  tests_vector_assign<trivially_copyable>();
  tests_vector_assign<nothrow_move_constructible>();
  tests_vector_assign<move_constructible>();
  tests_vector_assign<nothrow_copy_constructible>();
  tests_vector_assign<copy_constructible>();
  tests_vector_assignment_operator<trivially_copyable>();
  tests_vector_assignment_operator<nothrow_move_constructible>();
  tests_vector_assignment_operator<move_constructible>();
  tests_vector_assignment_operator<nothrow_copy_constructible>();
  tests_vector_assignment_operator<copy_constructible>();
  tests_vector_reserve<trivially_copyable>();
  tests_vector_reserve<nothrow_move_constructible>();
  tests_vector_reserve<move_constructible>();
  tests_vector_reserve<nothrow_copy_constructible>();
  tests_vector_reserve<copy_constructible>();
  tests_vector_shrink_to_fit<trivially_copyable>();
  tests_vector_shrink_to_fit<nothrow_move_constructible>();
  tests_vector_shrink_to_fit<move_constructible>();
  tests_vector_shrink_to_fit<nothrow_copy_constructible>();
  tests_vector_shrink_to_fit<copy_constructible>();
  tests_vector_clear<trivially_copyable>();
  tests_vector_clear<nothrow_move_constructible>();
  tests_vector_clear<move_constructible>();
  tests_vector_clear<nothrow_copy_constructible>();
  tests_vector_clear<copy_constructible>();
  tests_vector_insert<trivially_copyable>();
  tests_vector_insert<nothrow_move_constructible>();
  tests_vector_insert<move_constructible>();
  // tests_vector_insert<nothrow_copy_constructible>();  // Does not meet requirements
  // tests_vector_insert<copy_constructible>(); // Does not meet requirements
  tests_vector_emplace<trivially_copyable>();
  tests_vector_emplace<nothrow_move_constructible>();
  tests_vector_emplace<move_constructible>();
  // tests_vector_emplace<nothrow_copy_constructible>();  // Does not meet requirements
  // tests_vector_emplace<copy_constructible>();  // Does not meet requirements

  std::vector<int> x;
  x.get_allocator();
}
