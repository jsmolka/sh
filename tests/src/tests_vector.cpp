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

template <typename T, std::size_t N>
struct base {
  using vector = sh::vector<T, N>;

  static std::size_t capacity(std::size_t value) {
    return std::max<std::size_t>(value, N);
  }

  static auto member(std::string_view member) {
    return make_test("vector<{}, {}>::{}", typeid(T).name(), N, member);
  }
};

template <typename T, std::size_t N>
struct tests_constructor : public base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("vector()") = []() {
      vector vec1{};
      expect(vec1.size() == 0);
      expect(vec1.capacity() == N);
      expect(vec1.data() == nullptr);
      expect(vec1.begin() == nullptr);
      expect(vec1.end() == nullptr);
      expect(static_cast<bool>(vec1 == vector{}));
    };

    member("vector(size_type, const value_type&)") = []() {
      T v1{1};
      vector vec1(3, v1);
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{1, 1, 1}));

      vector vec2(0, v1);
      expect(vec2.size() == 0);
      expect(vec2.capacity() == N);
      expect(static_cast<bool>(vec2 == vector{}));
    };

    member("vector(size_type)") = []() {
      vector vec1(3);
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{0, 0, 0}));

      vector vec2(0);
      expect(vec2.size() == 0);
      expect(vec2.capacity() == N);
      expect(static_cast<bool>(vec2 == vector{}));
    };

    member("vector(Iterator, Iterator)") = []() {
      vector vec1{0, 1, 2};
      vector vec2(vec1.begin(), vec1.end());
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vec2));

      vector vec3{};
      vector vec4(vec3.begin(), vec3.end());
      expect(vec3.size() == 0);
      expect(vec3.capacity() == N);
      expect(static_cast<bool>(vec3 == vec4));
    };

    member("vector(const vector&)") = []() {
      vector vec1{0, 1, 2};
      vector vec2(vec1);
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vec2));
    };

    member("vector(vector&&)") = []() {
      vector vec1{0, 1, 2};
      const auto data = vec1.data();
      vector vec2(std::move(vec1));
      expect(vec1.data() == nullptr);
      expect(vec2.data() == data);
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec2 == vector{0, 1, 2}));
    };

    member("vector(initializer_list)") = []() {
      vector vec1{0, 1, 2};
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      expect(vec1[0] == T{0});
      expect(vec1[1] == T{1});
      expect(vec1[2] == T{2});
    };
  }
};

template <typename T, std::size_t N>
struct tests_comparison_operator : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("operator==") = []() {
      expect(static_cast<bool>(vector{0, 1, 2} == vector{0, 1, 2}));
      expect(!static_cast<bool>(vector{0, 1, 2} == vector{0, 1}));
      expect(!static_cast<bool>(vector{0, 1, 2} == vector{0, 1, 3}));
    };

    member("operator!=") = []() {
      expect(!static_cast<bool>(vector{0, 1, 2} != vector{0, 1, 2}));
      expect(static_cast<bool>(vector{0, 1, 2} != vector{0, 1}));
      expect(static_cast<bool>(vector{0, 1, 2} != vector{0, 1, 3}));
    };
  }
};

template <typename T, std::size_t N>
struct tests_assign : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("assign(size_type, const value_type&)") = []() {
      T v1{1};
      vector vec1{};
      vec1.assign(3, v1);
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{1, 1, 1}));

      vec1.assign(0, v1);
      expect(vec1.size() == 0);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{}));
    };

    member("assign(InputIt, InputIt)") = []() {
      vector vec1{0, 1, 2};
      vector vec2{};
      vec2.assign(vec1.begin(), vec1.end());
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{0, 1, 2}));

      vector vec3{};
      vec2.assign(vec3.begin(), vec3.end());
      expect(vec2.size() == 0);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec2 == vector{}));
    };

    member("assign(initializer_list)") = []() {
      vector vec1{};
      vec1.assign({0, 1, 2});
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{0, 1, 2}));

      vec1.assign({});
      expect(vec1.size() == 0);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{}));
    };
  }
};

template <typename T, std::size_t N>
struct tests_assignment_operator : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("operator=(const vector&)") = []() {
      vector vec1{0, 1, 2};
      vector vec2 = vec1;
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vec2));

      vector vec3{};
      vector vec4 = vec3;
      expect(vec4.size() == 0);
      expect(vec4.capacity() == capacity(0));
      expect(static_cast<bool>(vec3 == vec4));
    };

    member("operator=(vector&&)") = []() {
      vector vec1{0, 1, 2};
      const auto data = vec1.data();
      vector vec2 = std::move(vec1);
      expect(vec1.data() == nullptr);
      expect(vec2.data() == data);
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity(3));
      expect(static_cast<bool>(vec2 == vector{0, 1, 2}));
    };

    member("operator=(initializer_list)") = []() {
      vector vec1 = {0, 1, 2};
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{0, 1, 2}));

      vec1 = {};
      expect(vec1.size() == 0);
      expect(vec1.capacity() == capacity(3));
      expect(static_cast<bool>(vec1 == vector{}));
    };
  }
};

template <typename T, std::size_t N>
struct tests_reserve : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("reserve(size_type)") = []() {
      vector vec1{};
      for (auto reserve = 1; reserve <= 256; reserve *= 2) {
        vec1.reserve(reserve);
        expect(vec1.capacity() == capacity(reserve));
        vec1.reserve(reserve - 1);
        expect(vec1.capacity() == capacity(reserve));
      }
      expect(vec1.size() == 0);
    };
  }
};

template <typename T, std::size_t N>
struct tests_shrink_to_fit : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("shrink_to_fit()") = []() {
      vector vec1{};
      vec1.reserve(10);
      vec1.assign(5, {0});
      expect(vec1.size() == 5);
      expect(vec1.capacity() == capacity(10));
      vec1.shrink_to_fit();
      expect(vec1.size() == 5);
      expect(vec1.capacity() == capacity(5));

      vector vec2{};
      vec2.shrink_to_fit();
      expect(vec2.size() == 0);
      expect(vec2.capacity() == capacity(0));
    };
  }
};

template <typename T, std::size_t N>
struct tests_clear : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("clear()") = []() {
      vector vec1{0, 1, 2};
      expect(vec1.size() == 3);
      expect(vec1.capacity() == capacity(3));
      vec1.clear();
      expect(vec1.size() == 0);
      expect(vec1.capacity() == capacity(3));
    };
  }
};

template <typename T, std::size_t N>
struct tests_emplace : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::member;

  static void run() {
    member("emplace(const_iterator, Args...)") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T>) {
        vector v1{0, 0};
        v1.emplace(v1.begin(), 1);
        v1.emplace(v1.begin() + 2, 2);
        v1.emplace(v1.end(), 3);
        expect(static_cast<bool>(v1 == vector{1, 0, 2, 0, 3}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_insert : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::member;

  static void run() {
    member("insert(const_iterator, const value_type&|value_type&&)") = []() {
      vector v1{0, 0};
      v1.insert(v1.begin(), 1);
      v1.insert(v1.begin() + 2, 2);
      v1.insert(v1.end(), 3);
      expect(static_cast<bool>(v1 == vector{1, 0, 2, 0, 3}));
    };
  }
};

template <template <typename T, std::size_t N> typename Test, typename U>
void run() {
  Test<U, 0>::run();
}

template <template <typename T, std::size_t N> typename Test>
void run() {
  run<Test, int>();
  run<Test, trivially_copyable>();
  run<Test, nothrow_move_constructible>();
  run<Test, move_constructible>();
  run<Test, nothrow_copy_constructible>();
  run<Test, copy_constructible>();
}

void tests_vector() {
  run<tests_constructor>();
  run<tests_comparison_operator>();
  run<tests_assign>();
  run<tests_assignment_operator>();
  run<tests_reserve>();
  run<tests_shrink_to_fit>();
  run<tests_clear>();
  run<tests_insert, trivially_copyable>();
  run<tests_insert, nothrow_move_constructible>();
  run<tests_insert, move_constructible>();
  run<tests_emplace>();
}
