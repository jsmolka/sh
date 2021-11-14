#include "tests_vector.h"

#include <compare>

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
    };

    member("vector(size_type, const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(1);
        vector vec1(3, v1);
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1[0] == 1);
        expect(vec1[1] == 1);
        expect(vec1[2] == 1);

        vector vec2(0, v1);
        expect(vec2.size() == 0);
        expect(vec2.capacity() == N);
      }
    };

    member("vector(size_type)") = []() {
      if constexpr (sh::value_constructible<T>) {
        vector vec1(3);
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1[0] == 0);
        expect(vec1[1] == 0);
        expect(vec1[2] == 0);

        vector vec2(0);
        expect(vec2.size() == 0);
        expect(vec2.capacity() == N);
      }
    };

    member("vector(Iterator, Iterator)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2(vec1.begin(), vec1.end());
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(vec1[0] == vec2[0]);
        expect(vec1[1] == vec2[1]);
        expect(vec1[2] == vec2[2]);

        vector vec3{};
        vector vec4(vec3.begin(), vec3.end());
        expect(vec3.size() == 0);
        expect(vec3.capacity() == N);
      }
    };

    member("vector(const vector&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2(vec1);
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(vec1[0] == vec2[0]);
        expect(vec1[1] == vec2[1]);
        expect(vec1[2] == vec2[2]);
      }
    };

    member("vector(vector&&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        const auto data = vec1.data();
        vector vec2(std::move(vec1));
        expect(vec1.data() == nullptr);
        expect(vec2.data() == data);
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(vec2[0] == 0);
        expect(vec2[1] == 1);
        expect(vec2[2] == 2);
      }
    };

    member("vector(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1[0] == T{0});
        expect(vec1[1] == T{1});
        expect(vec1[2] == T{2});
      }
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
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);

      vector vec2{};
      vec2.emplace_back(0);
      vec2.emplace_back(1);
      vec2.emplace_back(2);

      expect(static_cast<bool>(vec1 == vec2));
      vec1.emplace_back(3);
      vec2.emplace_back(4);
      expect(!static_cast<bool>(vec1 == vec2));
    };

    member("operator!=") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);

      vector vec2{};
      vec2.emplace_back(0);
      vec2.emplace_back(1);

      expect(static_cast<bool>(vec1 != vec2));
      vec2.emplace_back(2);
      expect(!static_cast<bool>(vec1 != vec2));
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
      if constexpr (sh::copy_constructible<T>) {
        T v1{1};
        vector vec1{};
        vec1.assign(3, v1);
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1[0] == 1);
        expect(vec1[1] == 1);
        expect(vec1[2] == 1);

        vec1.assign(0, v1);
        expect(vec1.size() == 0);
        expect(vec1.capacity() == capacity(3));
      }
    };

    member("assign(InputIt, InputIt)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2{};
        vec2.assign(vec1.begin(), vec1.end());
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(vec1[0] == vec2[0]);
        expect(vec1[1] == vec2[1]);
        expect(vec1[2] == vec2[2]);

        vector vec3{};
        vec2.assign(vec3.begin(), vec3.end());
        expect(vec2.size() == 0);
        expect(vec2.capacity() == capacity(3));
      }
    };

    member("assign(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vec1.assign({0, 1, 2});
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1[0] == 0);
        expect(vec1[1] == 1);
        expect(vec1[2] == 2);

        vec1.assign({});
        expect(vec1.size() == 0);
        expect(vec1.capacity() == capacity(3));
      }
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
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2{};
        vec2 = vec1;
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(vec1[0] == vec2[0]);
        expect(vec1[1] == vec2[1]);
        expect(vec1[2] == vec2[2]);

        vector vec3{};
        vector vec4 = vec3;
        expect(vec4.size() == 0);
        expect(vec4.capacity() == capacity(0));

        vector vec5{0, 1, 2};
        const auto data = vec5.data();
        vec5 = vec5;
        expect(vec5.data() == data);
      }
    };

    member("operator=(vector&&)") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);
      const auto data = vec1.data();
      const auto capacity = vec1.capacity();
      vector vec2{};
      vec2 = std::move(vec1);
      expect(vec1.data() == nullptr);
      expect(vec2.data() == data);
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity);
      expect(vec2[0] == 0);
      expect(vec2[1] == 1);
      expect(vec2[2] == 2);

      vec2 = std::move(vec2);
      expect(vec2.data() == data);
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity);
    };

    member("operator=(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vec1 = {0, 1, 2};
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1[0] == 0);
        expect(vec1[1] == 1);
        expect(vec1[2] == 2);

        vec1 = {};
        expect(vec1.size() == 0);
        expect(vec1.capacity() == capacity(3));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_accessors : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::member;

  static void run() {
    member("begin()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(vec1.begin()[0] == 0);
      expect(vec1.cbegin()[0] == 0);
      expect(const_cast<const vector&>(vec1).begin()[0] == 0);
    };

    member("rbegin()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(vec1.rbegin()[0] == 1);
      expect(vec1.crbegin()[0] == 1);
      expect(const_cast<const vector&>(vec1).rbegin()[0] == 1);
    };

    member("end()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(vec1.end()[-1] == 1);
      expect(vec1.cend()[-1] == 1);
      expect(const_cast<const vector&>(vec1).end()[-1] == 1);
    };

    member("rend()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(vec1.rend()[-1] == 0);
      expect(vec1.crend()[-1] == 0);
      expect(const_cast<const vector&>(vec1).rend()[-1] == 0);
    };

    member("front()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(vec1.front() == 0);
      expect(const_cast<const vector&>(vec1).front() == 0);
    };

    member("end()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(vec1.back() == 1);
      expect(const_cast<const vector&>(vec1).back() == 1);
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
      for (auto reserve = 1; reserve <= 64; reserve *= 2) {
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
      while (vec1.size() < 8) {
        vec1.emplace_back(0);
        vec1.shrink_to_fit();
        expect(vec1.capacity() == capacity(vec1.size()));
      }
      expect(vec1.size() == 8);

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
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);
      expect(vec1.size() == 3);
      const auto capacity = vec1.capacity();
      vec1.clear();
      expect(vec1.size() == 0);
      expect(vec1.capacity() == capacity);
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
        vector vec1{0, 0};
        vec1.emplace(vec1.begin(), 1);
        vec1.emplace(vec1.begin() + 2, 2);
        vec1.emplace(vec1.end(), 3);
        expect(static_cast<bool>(vec1 == vector{1, 0, 2, 0, 3}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_insert : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("insert(const_iterator, const value_type&)") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        T v1(1);
        T v2(2);
        T v3(3);
        vector vec1{0, 0};
        vec1.insert(vec1.begin(), v1);
        vec1.insert(vec1.begin() + 2, v2);
        vec1.insert(vec1.end(), v3);
        expect(static_cast<bool>(vec1 == vector{1, 0, 2, 0, 3}));
      }
    };

    member("insert(const_iterator, value_type&&)") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T>) {
        T v1(1);
        T v2(2);
        T v3(3);
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(0);
        vec1.insert(vec1.begin(), std::move(v1));
        vec1.insert(vec1.begin() + 2, std::move(v2));
        vec1.insert(vec1.end(), std::move(v3));
        expect(vec1[0] == 1);
        expect(vec1[1] == 0);
        expect(vec1[2] == 2);
        expect(vec1[3] == 0);
        expect(vec1[4] == 3);
      }
    };

    member("insert(const_iterator, size_type, const value_type&") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        // count > size
        vector vec1(2, 0);
        auto pos = vec1.insert(vec1.end(), 3, 1);
        expect(pos == vec1.begin() + 2);
        expect(static_cast<bool>(vec1 == vector{0, 0, 1, 1, 1}));

        vector vec2(2, 0);
        pos = vec2.insert(vec2.begin(), 3, 1);
        expect(pos == vec2.begin());
        expect(static_cast<bool>(vec2 == vector{1, 1, 1, 0, 0}));

        vector vec3(2, 0);
        pos = vec3.insert(vec3.begin() + 1, 3, 1);
        expect(pos == vec3.begin() + 1);
        expect(static_cast<bool>(vec3 == vector{0, 1, 1, 1, 0}));

        // count < size
        vector vec4(3, 0);
        pos = vec4.insert(vec4.end(), 2, 1);
        expect(pos == vec4.begin() + 3);
        expect(static_cast<bool>(vec4 == vector{0, 0, 0, 1, 1}));

        vector vec5(3, 0);
        pos = vec5.insert(vec5.begin(), 2, 1);
        expect(pos == vec5.begin());
        expect(static_cast<bool>(vec5 == vector{1, 1, 0, 0, 0}));

        vector vec6(3, 0);
        pos = vec6.insert(vec6.begin() + 1, 2, 1);
        expect(pos == vec6.begin() + 1);
        expect(static_cast<bool>(vec6 == vector{0, 1, 1, 0, 0}));

        // count == size
        vector vec7(2, 0);
        pos = vec7.insert(vec7.end(), 2, 1);
        expect(pos == vec7.begin() + 2);
        expect(static_cast<bool>(vec7 == vector{0, 0, 1, 1}));

        vector vec8(2, 0);
        pos = vec8.insert(vec8.begin(), 2, 1);
        expect(pos == vec8.begin());
        expect(static_cast<bool>(vec8 == vector{1, 1, 0, 0}));

        vector vec9(2, 0);
        pos = vec9.insert(vec9.begin() + 1, 2, 1);
        expect(pos == vec9.begin() + 1);
        expect(static_cast<bool>(vec9 == vector{0, 1, 1, 0}));

        // count == 0
        vector vec10(2, 0);
        pos = vec10.insert(vec10.end(), 0, 1);
        expect(pos == vec10.end());
        expect(static_cast<bool>(vec10 == vector{0, 0}));
      }
    };

    member("insert(const_iterator, Iterator|std::initializer_list") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        // count > size
        vector vec1(2, 0);
        auto pos = vec1.insert(vec1.end(), {1, 2, 3});
        expect(pos == vec1.begin() + 2);
        expect(static_cast<bool>(vec1 == vector{0, 0, 1, 2, 3}));

        vector vec2(2, 0);
        pos = vec2.insert(vec2.begin(), {1, 2, 3});
        expect(pos == vec2.begin());
        expect(static_cast<bool>(vec2 == vector{1, 2, 3, 0, 0}));

        vector vec3(2, 0);
        pos = vec3.insert(vec3.begin() + 1, {1, 2, 3});
        expect(pos == vec3.begin() + 1);
        expect(static_cast<bool>(vec3 == vector{0, 1, 2, 3, 0}));

        // count < size
        vector vec4(3, 0);
        pos = vec4.insert(vec4.end(), {1, 2});
        expect(pos == vec4.begin() + 3);
        expect(static_cast<bool>(vec4 == vector{0, 0, 0, 1, 2}));

        vector vec5(3, 0);
        pos = vec5.insert(vec5.begin(), {1, 2});
        expect(pos == vec5.begin());
        expect(static_cast<bool>(vec5 == vector{1, 2, 0, 0, 0}));

        vector vec6(3, 0);
        pos = vec6.insert(vec6.begin() + 1, {1, 2});
        expect(pos == vec6.begin() + 1);
        expect(static_cast<bool>(vec6 == vector{0, 1, 2, 0, 0}));

        // count == size
        vector vec7(2, 0);
        pos = vec7.insert(vec7.end(), {1, 2});
        expect(pos == vec7.begin() + 2);
        expect(static_cast<bool>(vec7 == vector{0, 0, 1, 2}));

        vector vec8(2, 0);
        pos = vec8.insert(vec8.begin(), {1, 2});
        expect(pos == vec8.begin());
        expect(static_cast<bool>(vec8 == vector{1, 2, 0, 0}));

        vector vec9(2, 0);
        pos = vec9.insert(vec9.begin() + 1, {1, 2});
        expect(pos == vec9.begin() + 1);
        expect(static_cast<bool>(vec9 == vector{0, 1, 2, 0}));

        // count == 0
        vector vec10(2, 0);
        pos = vec10.insert(vec10.end(), {});
        expect(pos == vec10.end());
        expect(static_cast<bool>(vec10 == vector{0, 0}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_erase : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::member;

  static void run() {
    member("erase(const_iterator)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1(2, 0);
        auto pos = vec1.erase(vec1.begin());
        expect(pos == vec1.begin());
        expect(static_cast<bool>(vec1 == vector{0}));

        vector vec2(2, 0);
        pos = vec2.erase(vec2.begin() + 1);
        expect(pos == vec2.end());
        expect(static_cast<bool>(vec2 == vector{0}));
      }
    };

    member("erase(const_iterator, size_type)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{0, 1, 2};
        auto pos = vec1.erase(vec1.begin(), 2);
        expect(pos == vec1.begin());
        expect(static_cast<bool>(vec1 == vector{2}));

        vector vec2{0, 1, 2};
        pos = vec2.erase(vec2.begin() + 1, 2);
        expect(pos == vec2.end());
        expect(static_cast<bool>(vec2 == vector{0}));

        vector vec3{0, 1, 2};
        pos = vec3.erase(vec3.begin(), std::size_t(0));
        expect(pos == vec3.begin());
        expect(static_cast<bool>(vec3 == vector{0, 1, 2}));
      }
    };

    member("erase(const_iterator, const_iterator)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{0, 1, 2};
        auto pos = vec1.erase(vec1.begin(), vec1.begin() + 2);
        expect(pos == vec1.begin());
        expect(static_cast<bool>(vec1 == vector{2}));

        vector vec2{0, 1, 2};
        pos = vec2.erase(vec2.begin() + 1, vec2.end());
        expect(pos == vec2.end());
        expect(static_cast<bool>(vec2 == vector{0}));

        vector vec3{0, 1, 2};
        pos = vec3.erase(vec3.begin(), vec3.begin());
        expect(pos == vec3.begin());
        expect(static_cast<bool>(vec3 == vector{0, 1, 2}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_resize : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("resize(size_type, const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(1);
        vector vec1{};
        vec1.resize(5, v1);
        expect(vec1.size() == 5);
        expect(vec1.capacity() == capacity(5));
        expect(static_cast<bool>(vec1 == vector{1, 1, 1, 1, 1}));

        vec1.resize(2, v1);
        expect(vec1.size() == 2);
        expect(vec1.capacity() == capacity(5));
        expect(static_cast<bool>(vec1 == vector{1, 1}));
      }
    };

    member("resize(size_type)") = []() {
      if constexpr (std::default_initializable<T>) {
        vector vec1{};
        vec1.resize(5);
        expect(vec1.size() == 5);
        expect(vec1.capacity() == capacity(5));
        expect(static_cast<bool>(vec1 == vector{0, 0, 0, 0, 0}));

        vec1.resize(2);
        expect(vec1.size() == 2);
        expect(vec1.capacity() == capacity(5));
        expect(static_cast<bool>(vec1 == vector{0, 0}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_emplace_back : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("emplace_back(Args...)") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);
      expect(vec1.size() == 3);
      expect(vec1[0] == 0);
      expect(vec1[1] == 1);
      expect(vec1[2] == 2);

      if constexpr (sh::copy_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec2{};
        vec2.emplace_back(v1);
        vec2.emplace_back(v2);
        vec2.emplace_back(v3);
        expect(vec2.size() == 3);
        expect(vec2[0] == 0);
        expect(vec2[1] == 1);
        expect(vec2[2] == 2);
      }

      if constexpr (sh::move_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec3{};
        vec3.emplace_back(std::move(v1));
        vec3.emplace_back(std::move(v2));
        vec3.emplace_back(std::move(v3));
        expect(vec3.size() == 3);
        expect(vec3[0] == 0);
        expect(vec3[1] == 1);
        expect(vec3[2] == 2);
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_push_back : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("push_back(const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec2{};
        vec2.push_back(v1);
        vec2.push_back(v2);
        vec2.push_back(v3);
        expect(vec2.size() == 3);
        expect(vec2[0] == 0);
        expect(vec2[1] == 1);
        expect(vec2[2] == 2);
      }
    };

    member("push_back(value_type&&)") = []() {
      if constexpr (sh::move_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec3{};
        vec3.push_back(std::move(v1));
        vec3.push_back(std::move(v2));
        vec3.push_back(std::move(v3));
        expect(vec3.size() == 3);
        expect(vec3[0] == 0);
        expect(vec3[1] == 1);
        expect(vec3[2] == 2);
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_pop_back : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("pop_back()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      expect(vec1.size() == 1);
      vec1.emplace_back(0);
      expect(vec1.size() == 2);
      vec1.pop_back();
      expect(vec1.size() == 1);
      vec1.pop_back();
      expect(vec1.size() == 0);
    };
  }
};

template <typename T, std::size_t N>
struct tests_swap : base<T, N> {
  using typename base<T, N>::vector;
  using base<T, N>::capacity;
  using base<T, N>::member;

  static void run() {
    member("swap(const vector&)") = []() {
      vector vec1{0, 1, 2};
      vector vec2{};
      vec1.swap(vec2);
      expect(bool(vec1 == vector{}));
      expect(bool(vec2 == vector{0, 1, 2}));
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
  run<tests_accessors>();
  run<tests_reserve>();
  run<tests_shrink_to_fit>();
  run<tests_clear>();
  run<tests_emplace>();
  run<tests_insert>();
  run<tests_erase>();
  run<tests_resize>();
  run<tests_emplace_back>();
  run<tests_push_back>();
  run<tests_pop_back>();
  run<tests_swap>();
}
