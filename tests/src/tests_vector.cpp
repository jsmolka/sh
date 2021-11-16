#include "tests_vector.h"

#include <compare>

#include <sh/vector.h>

#include "ut.h"

struct vtest1 {
  vtest1() : vtest1(0) {}
  vtest1(int value) : value(value) {}
  vtest1(const vtest1&) noexcept = default;
  vtest1(vtest1&&) noexcept = default;
  vtest1& operator=(vtest1&&) noexcept = default;
  vtest1& operator=(const vtest1&) noexcept = default;
  auto operator<=>(const vtest1&) const = default;
  int value;
};

struct vtest2 {
  vtest2() : vtest2(0) {}
  vtest2(int value) : value(std::to_string(value)) {}
  vtest2(vtest2&&) noexcept = default;
  vtest2(const vtest2&) noexcept = default;
  vtest2& operator=(vtest2&&) noexcept = default;
  vtest2& operator=(const vtest2&) noexcept = default;
  auto operator<=>(const vtest2&) const = default;
  std::string value;
};

struct vtest3 {
  vtest3() : vtest3(0) {}
  vtest3(int value) : value(std::to_string(value)) {}
  vtest3(vtest3&&) noexcept(false) = default;
  vtest3(const vtest3&) noexcept = default;
  vtest3& operator=(vtest3&&) noexcept = default;
  vtest3& operator=(const vtest3&) noexcept = default;
  auto operator<=>(const vtest3&) const = default;
  std::string value;
};

struct vtest4 {
  vtest4() : vtest4(0) {}
  vtest4(int value) : value(std::to_string(value)) {}
  vtest4(vtest4&&) = delete;
  vtest4(const vtest4&) noexcept = default;
  vtest4& operator=(vtest4&&) = default;
  vtest4& operator=(const vtest4&) = default;
  auto operator<=>(const vtest4&) const = default;
  std::string value;
};

struct vtest5 {
  vtest5() : vtest5(0) {}
  vtest5(int value) : value(std::to_string(value)) {}
  vtest5(vtest5&&) = delete;
  vtest5(const vtest5&) noexcept(false) = default;
  vtest5& operator=(vtest5&&) = default;
  vtest5& operator=(const vtest5&) = default;
  auto operator<=>(const vtest5&) const = default;
  std::string value;
};

struct vtest6 {
  vtest6() : vtest6(0) {}
  vtest6(int value) : value(std::to_string(value)) {}
  vtest6(vtest6&&) noexcept = default;
  vtest6(const vtest6&) = delete;
  vtest6& operator=(vtest6&&) = default;
  vtest6& operator=(const vtest6&) = default;
  auto operator<=>(const vtest6&) const = default;
  std::string value;
};

template <typename T>
concept vector_test_type = requires(T&& t) {
  t.value;
};

template <vector_test_type T>
std::ostream& operator<<(std::ostream& out, const T& value) {
  out << value.value;
  return out;
}

template <typename T, std::size_t kSize>
std::ostream& operator<<(std::ostream& out, const sh::vector<T, kSize>& vec) {
  out << "[";
  for (const auto& value : vec) {
    out << value;
    if (&value != vec.end()) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

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
      if constexpr (N == 0) {
        expect(vec1.data() == nullptr);
        expect(vec1.begin() == nullptr);
        expect(vec1.end() == nullptr);
      }
    };

    member("vector(size_type, const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(1);
        vector vec1(3, v1);
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(vec1 == vector{1, 1, 1});

        vector vec2(0, v1);
        expect(vec2.size() == 0);
        expect(vec2.capacity() == N);
        expect(vec2 == vector{});
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
        expect(vec1 == vec2);

        vector vec3{};
        vector vec4(vec3.begin(), vec3.end());
        expect(vec3.size() == 0);
        expect(vec3.capacity() == N);
        expect(bool(vec3 == vec4));
      }
    };

    member("vector(const vector&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2(vec1);
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(bool(vec1 == vec2));
      }
    };

    member("vector(vector&&)") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);
      const auto data = vec1.data();
      const auto capacity = vec1.capacity();

      vector vec2(std::move(vec1));
      expect(vec1.data() == nullptr);
      if constexpr (N < 3) {
        expect(vec2.data() == data);
      }
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity);
      expect(vec2[0] == 0);
      expect(vec2[1] == 1);
      expect(vec2[2] == 2);
    };

    member("vector(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(bool(vec1 == vector{0, 1, 2}));
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

      expect(bool(vec1 == vec2));
      vec1.emplace_back(3);
      vec2.emplace_back(4);
      expect(!bool(vec1 == vec2));
    };

    member("operator!=") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);

      vector vec2{};
      vec2.emplace_back(0);
      vec2.emplace_back(1);

      expect(bool(vec1 != vec2));
      vec2.emplace_back(2);
      expect(!bool(vec1 != vec2));
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
        expect(bool(vec1 == vector{1, 1, 1}));

        vec1.assign(0, v1);
        expect(vec1.size() == 0);
        expect(vec1.capacity() == capacity(3));
        expect(bool(vec1 == vector{}));
      }
    };

    member("assign(InputIt, InputIt)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2{};
        vec2.assign(vec1.begin(), vec1.end());
        expect(vec2.size() == 3);
        expect(vec2.capacity() == capacity(3));
        expect(bool(vec1 == vec2));

        vector vec3{};
        vec2.assign(vec3.begin(), vec3.end());
        expect(vec2.size() == 0);
        expect(vec2.capacity() == capacity(3));
        expect(bool(vec2 == vec3));
      }
    };

    member("assign(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vec1.assign({0, 1, 2});
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(bool(vec1 == vector{0, 1, 2}));

        vec1.assign({});
        expect(vec1.size() == 0);
        expect(vec1.capacity() == capacity(3));
        expect(bool(vec1 == vector{}));
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
        expect(bool(vec1 == vec2));

        vector vec3{};
        vector vec4 = vec3;
        expect(vec4.size() == 0);
        expect(vec4.capacity() == capacity(0));
        expect(bool(vec3 == vec4));

        vector vec5{0, 1, 2};
        const auto data = vec5.data();
        vec5 = vec5;
        expect(vec5.data() == data);
        expect(bool(vec5 == vector{0, 1, 2}));
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
      if constexpr (N < 3) {
        expect(eq(vec2.data(), data));
      }
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity);
      expect(vec2[0] == 0);
      expect(vec2[1] == 1);
      expect(vec2[2] == 2);

      vec2 = std::move(vec2);
      if constexpr (N < 3) {
        expect(eq(vec2.data(), data));
      }
      expect(vec2.size() == 3);
      expect(vec2.capacity() == capacity);
    };

    member("operator=(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vec1 = {0, 1, 2};
        expect(vec1.size() == 3);
        expect(vec1.capacity() == capacity(3));
        expect(bool(vec1 == vector{0, 1, 2}));

        vec1 = {};
        expect(vec1.size() == 0);
        expect(vec1.capacity() == capacity(3));
        expect(bool(vec1 == vector{}));
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

    member("back()") = []() {
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
        expect(vec1[0] == 1);
        expect(vec1[1] == 0);
        expect(vec1[2] == 2);
        expect(vec1[3] == 0);
        expect(vec1[4] == 3);
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
        expect(bool(vec1 == vector{1, 0, 2, 0, 3}));
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
        expect(bool(vec1 == vector{0, 0, 1, 1, 1}));

        vector vec2(2, 0);
        pos = vec2.insert(vec2.begin(), 3, 1);
        expect(pos == vec2.begin());
        expect(bool(vec2 == vector{1, 1, 1, 0, 0}));

        vector vec3(2, 0);
        pos = vec3.insert(vec3.begin() + 1, 3, 1);
        expect(pos == vec3.begin() + 1);
        expect(bool(vec3 == vector{0, 1, 1, 1, 0}));

        // count < size
        vector vec4(3, 0);
        pos = vec4.insert(vec4.end(), 2, 1);
        expect(pos == vec4.begin() + 3);
        expect(bool(vec4 == vector{0, 0, 0, 1, 1}));

        vector vec5(3, 0);
        pos = vec5.insert(vec5.begin(), 2, 1);
        expect(pos == vec5.begin());
        expect(bool(vec5 == vector{1, 1, 0, 0, 0}));

        vector vec6(3, 0);
        pos = vec6.insert(vec6.begin() + 1, 2, 1);
        expect(pos == vec6.begin() + 1);
        expect(bool(vec6 == vector{0, 1, 1, 0, 0}));

        // count == size
        vector vec7(2, 0);
        pos = vec7.insert(vec7.end(), 2, 1);
        expect(pos == vec7.begin() + 2);
        expect(bool(vec7 == vector{0, 0, 1, 1}));

        vector vec8(2, 0);
        pos = vec8.insert(vec8.begin(), 2, 1);
        expect(pos == vec8.begin());
        expect(bool(vec8 == vector{1, 1, 0, 0}));

        vector vec9(2, 0);
        pos = vec9.insert(vec9.begin() + 1, 2, 1);
        expect(pos == vec9.begin() + 1);
        expect(bool(vec9 == vector{0, 1, 1, 0}));

        // count == 0
        vector vec10(2, 0);
        pos = vec10.insert(vec10.end(), 0, 1);
        expect(pos == vec10.end());
        expect(bool(vec10 == vector{0, 0}));
      }
    };

    member("insert(const_iterator, Iterator|std::initializer_list") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        // count > size
        vector vec1(2, 0);
        auto pos = vec1.insert(vec1.end(), {1, 2, 3});
        expect(pos == vec1.begin() + 2);
        expect(bool(vec1 == vector{0, 0, 1, 2, 3}));

        vector vec2(2, 0);
        pos = vec2.insert(vec2.begin(), {1, 2, 3});
        expect(pos == vec2.begin());
        expect(bool(vec2 == vector{1, 2, 3, 0, 0}));

        vector vec3(2, 0);
        pos = vec3.insert(vec3.begin() + 1, {1, 2, 3});
        expect(pos == vec3.begin() + 1);
        expect(bool(vec3 == vector{0, 1, 2, 3, 0}));

        // count < size
        vector vec4(3, 0);
        pos = vec4.insert(vec4.end(), {1, 2});
        expect(pos == vec4.begin() + 3);
        expect(bool(vec4 == vector{0, 0, 0, 1, 2}));

        vector vec5(3, 0);
        pos = vec5.insert(vec5.begin(), {1, 2});
        expect(pos == vec5.begin());
        expect(bool(vec5 == vector{1, 2, 0, 0, 0}));

        vector vec6(3, 0);
        pos = vec6.insert(vec6.begin() + 1, {1, 2});
        expect(pos == vec6.begin() + 1);
        expect(bool(vec6 == vector{0, 1, 2, 0, 0}));

        // count == size
        vector vec7(2, 0);
        pos = vec7.insert(vec7.end(), {1, 2});
        expect(pos == vec7.begin() + 2);
        expect(bool(vec7 == vector{0, 0, 1, 2}));

        vector vec8(2, 0);
        pos = vec8.insert(vec8.begin(), {1, 2});
        expect(pos == vec8.begin());
        expect(bool(vec8 == vector{1, 2, 0, 0}));

        vector vec9(2, 0);
        pos = vec9.insert(vec9.begin() + 1, {1, 2});
        expect(pos == vec9.begin() + 1);
        expect(bool(vec9 == vector{0, 1, 2, 0}));

        // count == 0
        vector vec10(2, 0);
        pos = vec10.insert(vec10.end(), {});
        expect(pos == vec10.end());
        expect(bool(vec10 == vector{0, 0}));
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
        expect(bool(vec1 == vector{0}));

        vector vec2(2, 0);
        pos = vec2.erase(vec2.begin() + 1);
        expect(pos == vec2.end());
        expect(bool(vec2 == vector{0}));
      }
    };

    member("erase(const_iterator, size_type)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{0, 1, 2};
        auto pos = vec1.erase(vec1.begin(), 2);
        expect(pos == vec1.begin());
        expect(bool(vec1 == vector{2}));

        vector vec2{0, 1, 2};
        pos = vec2.erase(vec2.begin() + 1, 2);
        expect(pos == vec2.end());
        expect(bool(vec2 == vector{0}));

        vector vec3{0, 1, 2};
        pos = vec3.erase(vec3.begin(), std::size_t(0));
        expect(pos == vec3.begin());
        expect(bool(vec3 == vector{0, 1, 2}));
      }
    };

    member("erase(const_iterator, const_iterator)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{0, 1, 2};
        auto pos = vec1.erase(vec1.begin(), vec1.begin() + 2);
        expect(pos == vec1.begin());
        expect(bool(vec1 == vector{2}));

        vector vec2{0, 1, 2};
        pos = vec2.erase(vec2.begin() + 1, vec2.end());
        expect(pos == vec2.end());
        expect(bool(vec2 == vector{0}));

        vector vec3{0, 1, 2};
        pos = vec3.erase(vec3.begin(), vec3.begin());
        expect(pos == vec3.begin());
        expect(bool(vec3 == vector{0, 1, 2}));
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
        expect(eq(vec1.size(), 5));
        expect(eq(vec1.capacity(), capacity(5)));
        expect(bool(vec1 == vector{1, 1, 1, 1, 1}));

        vec1.resize(2, v1);
        expect(vec1.size() == 2);
        expect(vec1.capacity() == capacity(5));
        expect(bool(vec1 == vector{1, 1}));
      }
    };

    member("resize(size_type)") = []() {
      if constexpr (sh::value_constructible<T>) {
        vector vec1{};
        vec1.resize(5);
        expect(eq(vec1.size(), 5));
        expect(eq(vec1.capacity(), capacity(5)));
        expect(bool(vec1 == vector{0, 0, 0, 0, 0}));

        vec1.resize(2);
        expect(vec1.size() == 2);
        expect(vec1.capacity() == capacity(5));
        expect(bool(vec1 == vector{0, 0}));
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
        expect(bool(vec2 == vector{0, 1, 2}));
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
        expect(bool(vec2 == vector{0, 1, 2}));
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
      if constexpr (N == 0 || sh::copy_constructible<T>) {
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(1);
        vec1.emplace_back(2);
        vector vec2{};
        vec1.swap(vec2);

        vector vec3{};
        vector vec4{};
        vec4.emplace_back(0);
        vec4.emplace_back(1);
        vec4.emplace_back(2);

        expect(bool(vec1 == vec3));
        // expect(bool(vec2 == vec4));
      }
    };
  }
};

template <template <typename T, std::size_t N> typename Test, typename U>
void run() {
  Test<U, 0>::run();
  Test<U, 1>::run();
  Test<U, 2>::run();
  Test<U, 4>::run();
  Test<U, 8>::run();
}

template <template <typename T, std::size_t N> typename Test>
void run() {
  run<Test, int>();
  run<Test, vtest1>();
  run<Test, vtest2>();
  run<Test, vtest3>();
  run<Test, vtest4>();
  run<Test, vtest5>();
  // run<Test, vtest6>();
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
