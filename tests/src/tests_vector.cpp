#include "tests_vector.h"

#include "common.h"
#include "common_types.h"

template <typename T, std::size_t N>
auto operator<<(std::ostream& out, const sh::vector<T, N>& vector) -> std::ostream& {
  return out << fmt::format("[{}]", fmt::join(vector, ", "));
}

namespace {

template <typename T, std::size_t N>
struct tests {
  using vector = sh::vector<T, N>;

  static auto capacity(std::size_t value) -> std::size_t {
    return std::max(value, N);
  }

  static auto test(std::string_view what) {
    return sh::test("vector<{}, {}>::{}", typeid(T).name(), N, what);
  }
};

template <typename T, std::size_t N>
struct tests_constructor : public tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("vector()") = []() {
      vector vec1{};
      expect(eq(vec1.size(), 0));
      expect(eq(vec1.capacity(), N));
      if constexpr (N == 0) {
        expect(eq(vec1.data(), nullptr));
        expect(eq(vec1.begin(), nullptr));
        expect(eq(vec1.end(), nullptr));
      } else {
        expect(neq(vec1.data(), nullptr));
        expect(neq(vec1.begin(), nullptr));
        expect(neq(vec1.end(), nullptr));
      }
    };

    test("vector(size_type, const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(1);
        vector vec1(3, v1);
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{1, 1, 1}));

        vector vec2(0, v1);
        expect(eq(vec2.size(), 0));
        expect(eq(vec2.capacity(), N));
        expect(eq(vec2, vector{}));
      }
    };

    test("vector(size_type)") = []() {
      if constexpr (std::default_initializable<T>) {
        vector vec1(3);
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1[0], 0));
        expect(eq(vec1[1], 0));
        expect(eq(vec1[2], 0));

        vector vec2(0);
        expect(eq(vec2.size(), 0));
        expect(eq(vec2.capacity(), N));
      }
    };

    test("vector(Iterator, Iterator)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2(vec1.begin(), vec1.end());
        expect(eq(vec2.size(), 3));
        expect(eq(vec2.capacity(), capacity(3)));
        expect(eq(vec1, vec2));

        vector vec3{};
        vector vec4(vec3.begin(), vec3.end());
        expect(eq(vec3.size(), 0));
        expect(eq(vec3.capacity(), N));
        expect(eq(vec3, vec4));
      }
    };

    test("vector(const vector&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        vector vec2(vec1);
        expect(eq(vec2.size(), 3));
        expect(eq(vec2.capacity(), capacity(3)));
        expect(eq(vec1, vec2));

        vector vec3{};
        vector vec4(vec3);
        expect(eq(vec4.size(), 0));
        expect(eq(vec4.capacity(), N));
        expect(eq(vec3, vec4));
      }
    };

    test("vector(vector&&)") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);
      const auto d = vec1.data();
      const auto c = vec1.capacity();

      vector vec2(std::move(vec1));
      if constexpr (N == 0) {
        expect(eq(vec1.data(), nullptr));
      } else {
        expect(neq(vec1.data(), nullptr));
      }
      expect(eq(vec2.size(), 3));
      expect(eq(vec2[0], 0));
      expect(eq(vec2[1], 1));
      expect(eq(vec2[2], 2));

      if constexpr (N < 3) {
        expect(eq(vec2.data(), d));
        expect(eq(vec2.capacity(), c));
      }
    };

    test("vector(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{0, 1, 2};
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{0, 1, 2}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_comparison_operator : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("operator==") = []() {
      vector vec1{};
      vector vec2{};
      expect(eq(vec1, vec2));

      vec1.emplace_back(0);
      vec1.emplace_back(1);
      expect(neq(vec1, vec2));

      vec2.emplace_back(0);
      vec2.emplace_back(1);
      expect(eq(vec1, vec2));

      vec1.emplace_back(2);
      vec2.emplace_back(3);
      expect(neq(vec1, vec2));
    };
  }
};

template <typename T, std::size_t N>
struct tests_assign : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("assign(size_type, const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1{1};
        vector vec1{};
        vec1.assign(3, v1);
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{1, 1, 1}));

        vec1.assign(0, v1);
        expect(eq(vec1.size(), 0));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{}));
      }
    };

    test("assign(InputIt, InputIt)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vector vec2{0, 1, 2};
        vec1.assign(vec2.begin(), vec2.end());
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vec2));

        vector vec3{};
        vector vec4{};
        vec3.assign(vec4.begin(), vec4.end());
        expect(eq(vec3.size(), 0));
        expect(eq(vec3.capacity(), capacity(N)));
        expect(eq(vec3, vec4));
      }
    };

    test("assign(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vec1.assign({0, 1, 2});
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{0, 1, 2}));

        vec1.assign({});
        expect(eq(vec1.size(), 0));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_assignment_operator : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("operator=(const vector&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vector vec2{0, 1, 2};
        vec1 = vec2;
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vec2));

        vector vec3{};
        vector vec4{};
        vec3 = vec4;
        expect(eq(vec4.size(), 0));
        expect(eq(vec4.capacity(), capacity(0)));
        expect(eq(vec3, vec4));

        vector vec5{0, 1, 2};
        const auto d = vec5.data();
        vec5 = vec5;
        expect(eq(vec5.data(), d));
        expect(eq(vec5, vector{0, 1, 2}));
      }
    };

    test("operator=(vector&&)") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);
      const auto d = vec1.data();
      const auto c = vec1.capacity();

      vector vec2{};
      vec2 = std::move(vec1);
      if constexpr (N == 0) {
        expect(eq(vec1.data(), nullptr));
      } else {
        expect(neq(vec1.data(), nullptr));
      }
      expect(eq(vec2.size(), 3));
      expect(eq(vec2[0], 0));
      expect(eq(vec2[1], 1));
      expect(eq(vec2[2], 2));

      if constexpr (N < 3) {
        expect(eq(vec2.data(), d));
        expect(eq(vec2.capacity(), c));
      }

      vector vec3{};
      vec3.emplace_back(3);
      vec3.emplace_back(3);
      vec3.emplace_back(3);
      vec3.emplace_back(3);
      vec3 = std::move(vec2);
      if constexpr (N == 0) {
        expect(eq(vec2.data(), nullptr));
      } else {
        expect(neq(vec2.data(), nullptr));
      }
      expect(eq(vec3.size(), 3));
      expect(eq(vec3[0], 0));
      expect(eq(vec3[1], 1));
      expect(eq(vec3[2], 2));

      if constexpr (N < 3) {
        expect(eq(vec3.data(), d));
        expect(eq(vec3.capacity(), c));
      }
    };

    test("operator=(initializer_list)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        vector vec1{};
        vec1 = {0, 1, 2};
        expect(eq(vec1.size(), 3));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{0, 1, 2}));

        vec1 = {};
        expect(eq(vec1.size(), 0));
        expect(eq(vec1.capacity(), capacity(3)));
        expect(eq(vec1, vector{}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_accessors : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::test;

  static void run() {
    test("begin()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(eq(vec1.begin()[0], 0));
      expect(eq(vec1.cbegin()[0], 0));
      expect(eq(const_cast<const vector&>(vec1).begin()[0], 0));
      expect(eq(const_cast<const vector&>(vec1).cbegin()[0], 0));
    };

    test("end()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(eq(vec1.end()[-1], 1));
      expect(eq(vec1.cend()[-1], 1));
      expect(eq(const_cast<const vector&>(vec1).end()[-1], 1));
      expect(eq(const_cast<const vector&>(vec1).cend()[-1], 1));
    };

    test("rbegin()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(eq(vec1.rbegin()[0], 1));
      expect(eq(vec1.crbegin()[0], 1));
      expect(eq(const_cast<const vector&>(vec1).rbegin()[0], 1));
      expect(eq(const_cast<const vector&>(vec1).crbegin()[0], 1));
    };

    test("rend()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(eq(vec1.rend()[-1], 0));
      expect(eq(vec1.crend()[-1], 0));
      expect(eq(const_cast<const vector&>(vec1).rend()[-1], 0));
      expect(eq(const_cast<const vector&>(vec1).crend()[-1], 0));
    };

    test("front()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(eq(vec1.front(), 0));
      expect(eq(const_cast<const vector&>(vec1).front(), 0));
    };

    test("back()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);

      expect(eq(vec1.back(), 1));
      expect(eq(const_cast<const vector&>(vec1).back(), 1));
    };
  }
};

template <typename T, std::size_t N>
struct tests_reserve : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("reserve(size_type)") = []() {
      vector vec1{};
      for (std::size_t reserve = 1; reserve < 32; reserve <<= 2) {
        vec1.reserve(reserve);
        expect(eq(vec1.capacity(), capacity(reserve)));
        vec1.reserve(reserve - 1);
        expect(eq(vec1.capacity(), capacity(reserve)));
        vec1.reserve(0);
        expect(eq(vec1.capacity(), capacity(reserve)));
      }
      expect(eq(vec1.size(), 0));
    };
  }
};

template <typename T, std::size_t N>
struct tests_shrink_to_fit : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("shrink_to_fit()") = []() {
      vector vec1{};
      while (vec1.size() < 8) {
        vec1.emplace_back(0);
        vec1.shrink_to_fit();
        expect(eq(vec1.capacity(), capacity(vec1.size())));
      }
      expect(eq(vec1.size(), 8));

      vector vec2{};
      vec2.shrink_to_fit();
      expect(eq(vec2.size(), 0));
      expect(eq(vec2.capacity(), capacity(0)));
    };
  }
};

template <typename T, std::size_t N>
struct tests_clear : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("clear()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(1);
      vec1.emplace_back(2);

      const auto c = vec1.capacity();
      expect(eq(vec1.size(), 3));
      expect(eq(vec1.capacity(), c));
      vec1.clear();
      expect(eq(vec1.size(), 0));
      expect(eq(vec1.capacity(), c));
    };
  }
};

template <typename T, std::size_t N>
struct tests_emplace : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::test;

  static void run() {
    test("emplace(const_iterator, Args...)") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T>) {
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(0);
        expect(eq(*vec1.emplace(vec1.begin(), 1), 1));
        expect(eq(*vec1.emplace(vec1.begin() + 2, 2), 2));
        expect(eq(*vec1.emplace(vec1.end(), 3), 3));
        expect(eq(vec1[0], 1));
        expect(eq(vec1[1], 0));
        expect(eq(vec1[2], 2));
        expect(eq(vec1[3], 0));
        expect(eq(vec1[4], 3));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_insert : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("insert(const_iterator, const value_type&)") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        T v1(1);
        T v2(2);
        T v3(3);
        vector vec1{0, 0};
        expect(eq(*vec1.insert(vec1.begin(), v1), 1));
        expect(eq(*vec1.insert(vec1.begin() + 2, v2), 2));
        expect(eq(*vec1.insert(vec1.end(), v3), 3));
        expect(eq(vec1, vector{1, 0, 2, 0, 3}));
      }
    };

    test("insert(const_iterator, value_type&&)") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T>) {
        T v1(1);
        T v2(2);
        T v3(3);
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(0);
        expect(eq(*vec1.insert(vec1.begin(), std::move(v1)), 1));
        expect(eq(*vec1.insert(vec1.begin() + 2, std::move(v2)), 2));
        expect(eq(*vec1.insert(vec1.end(), std::move(v3)), 3));
        expect(eq(vec1[0], 1));
        expect(eq(vec1[1], 0));
        expect(eq(vec1[2], 2));
        expect(eq(vec1[3], 0));
        expect(eq(vec1[4], 3));
      }
    };

    test("insert(const_iterator, size_type, const value_type&") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        // count > size
        vector vec1(2, 0);
        auto pos = vec1.insert(vec1.end(), 3, 1);
        expect(eq(pos, vec1.begin() + 2));
        expect(eq(vec1, vector{0, 0, 1, 1, 1}));

        vector vec2(2, 0);
        pos = vec2.insert(vec2.begin(), 3, 1);
        expect(eq(pos, vec2.begin()));
        expect(eq(vec2, vector{1, 1, 1, 0, 0}));

        vector vec3(2, 0);
        pos = vec3.insert(vec3.begin() + 1, 3, 1);
        expect(eq(pos, vec3.begin() + 1));
        expect(eq(vec3, vector{0, 1, 1, 1, 0}));

        // count < size
        vector vec4(3, 0);
        pos = vec4.insert(vec4.end(), 2, 1);
        expect(eq(pos, vec4.begin() + 3));
        expect(eq(vec4, vector{0, 0, 0, 1, 1}));

        vector vec5(3, 0);
        pos = vec5.insert(vec5.begin(), 2, 1);
        expect(eq(pos, vec5.begin()));
        expect(eq(vec5, vector{1, 1, 0, 0, 0}));

        vector vec6(3, 0);
        pos = vec6.insert(vec6.begin() + 1, 2, 1);
        expect(eq(pos, vec6.begin() + 1));
        expect(eq(vec6, vector{0, 1, 1, 0, 0}));

        // count == size
        vector vec7(2, 0);
        pos = vec7.insert(vec7.end(), 2, 1);
        expect(eq(pos, vec7.begin() + 2));
        expect(eq(vec7, vector{0, 0, 1, 1}));

        vector vec8(2, 0);
        pos = vec8.insert(vec8.begin(), 2, 1);
        expect(eq(pos, vec8.begin()));
        expect(eq(vec8, vector{1, 1, 0, 0}));

        vector vec9(2, 0);
        pos = vec9.insert(vec9.begin() + 1, 2, 1);
        expect(eq(pos, vec9.begin() + 1));
        expect(eq(vec9, vector{0, 1, 1, 0}));

        // count == 0
        vector vec10(2, 0);
        pos = vec10.insert(vec10.end(), 0, 1);
        expect(eq(pos, vec10.end()));
        expect(eq(vec10, vector{0, 0}));
      }
    };

    test("insert(const_iterator, Iterator|std::initializer_list") = []() {
      if constexpr (sh::move_constructible<T> && sh::move_assignable<T> &&
                    sh::copy_constructible<T>) {
        // count > size
        vector vec1(2, 0);
        auto pos = vec1.insert(vec1.end(), {1, 2, 3});
        expect(eq(pos, vec1.begin() + 2));
        expect(eq(vec1, vector{0, 0, 1, 2, 3}));

        vector vec2(2, 0);
        pos = vec2.insert(vec2.begin(), {1, 2, 3});
        expect(eq(pos, vec2.begin()));
        expect(eq(vec2, vector{1, 2, 3, 0, 0}));

        vector vec3(2, 0);
        pos = vec3.insert(vec3.begin() + 1, {1, 2, 3});
        expect(eq(pos, vec3.begin() + 1));
        expect(eq(vec3, vector{0, 1, 2, 3, 0}));

        // count < size
        vector vec4(3, 0);
        pos = vec4.insert(vec4.end(), {1, 2});
        expect(eq(pos, vec4.begin() + 3));
        expect(eq(vec4, vector{0, 0, 0, 1, 2}));

        vector vec5(3, 0);
        pos = vec5.insert(vec5.begin(), {1, 2});
        expect(eq(pos, vec5.begin()));
        expect(eq(vec5, vector{1, 2, 0, 0, 0}));

        vector vec6(3, 0);
        pos = vec6.insert(vec6.begin() + 1, {1, 2});
        expect(eq(pos, vec6.begin() + 1));
        expect(eq(vec6, vector{0, 1, 2, 0, 0}));

        // count == size
        vector vec7(2, 0);
        pos = vec7.insert(vec7.end(), {1, 2});
        expect(eq(pos, vec7.begin() + 2));
        expect(eq(vec7, vector{0, 0, 1, 2}));

        vector vec8(2, 0);
        pos = vec8.insert(vec8.begin(), {1, 2});
        expect(eq(pos, vec8.begin()));
        expect(eq(vec8, vector{1, 2, 0, 0}));

        vector vec9(2, 0);
        pos = vec9.insert(vec9.begin() + 1, {1, 2});
        expect(eq(pos, vec9.begin() + 1));
        expect(eq(vec9, vector{0, 1, 2, 0}));

        // count == 0
        vector vec10(2, 0);
        pos = vec10.insert(vec10.end(), {});
        expect(eq(pos, vec10.end()));
        expect(eq(vec10, vector{0, 0}));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_erase : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::test;

  static void run() {
    test("erase(const_iterator)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(0);
        auto pos = vec1.erase(vec1.begin());
        expect(eq(pos, vec1.begin()));
        expect(eq(vec1.size(), 1));
        expect(eq(vec1[0], 0));

        vector vec2{};
        vec2.emplace_back(0);
        vec2.emplace_back(0);
        pos = vec2.erase(vec2.begin() + 1);
        expect(eq(pos, vec2.end()));
        expect(eq(vec2.size(), 1));
        expect(eq(vec2[0], 0));
      }
    };

    test("erase(const_iterator, size_type)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(1);
        vec1.emplace_back(2);
        auto pos = vec1.erase(vec1.begin(), 2);
        expect(eq(pos, vec1.begin()));
        expect(eq(vec1.size(), 1));
        expect(eq(vec1[0], 2));

        vector vec2{};
        vec2.emplace_back(0);
        vec2.emplace_back(1);
        vec2.emplace_back(2);
        pos = vec2.erase(vec2.begin() + 1, 2);
        expect(eq(pos, vec2.end()));
        expect(eq(vec2.size(), 1));
        expect(eq(vec2[0], 0));

        vector vec3{};
        vec3.emplace_back(0);
        vec3.emplace_back(1);
        vec3.emplace_back(2);
        pos = vec3.erase(vec3.begin(), std::size_t(0));
        expect(eq(pos, vec3.begin()));
        expect(eq(vec3.size(), 3));
        expect(eq(vec3[0], 0));
        expect(eq(vec3[1], 1));
        expect(eq(vec3[2], 2));
      }
    };

    test("erase(const_iterator, const_iterator)") = []() {
      if constexpr (sh::move_assignable<T>) {
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(1);
        vec1.emplace_back(2);
        auto pos = vec1.erase(vec1.begin(), vec1.begin() + 2);
        expect(eq(pos, vec1.begin()));
        expect(eq(vec1.size(), 1));
        expect(eq(vec1[0], 2));

        vector vec2{};
        vec2.emplace_back(0);
        vec2.emplace_back(1);
        vec2.emplace_back(2);
        pos = vec2.erase(vec2.begin() + 1, vec2.end());
        expect(eq(pos, vec2.end()));
        expect(eq(vec2.size(), 1));
        expect(eq(vec2[0], 0));

        vector vec3{};
        vec2.emplace_back(0);
        vec2.emplace_back(1);
        vec2.emplace_back(2);
        pos = vec3.erase(vec3.begin(), vec3.begin());
        expect(eq(pos, vec3.begin()));
        expect(eq(vec3.size(), 0));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_resize : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("resize(size_type, const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(1);
        vector vec1{};
        vec1.resize(5, v1);
        expect(eq(vec1.size(), 5));
        expect(eq(vec1.capacity(), capacity(5)));
        expect(eq(vec1, vector{1, 1, 1, 1, 1}));

        vec1.resize(2, v1);
        expect(eq(vec1.size(), 2));
        expect(eq(vec1.capacity(), capacity(5)));
        expect(eq(vec1, vector{1, 1}));
      }
    };

    test("resize(size_type)") = []() {
      if constexpr (std::default_initializable<T>) {
        vector vec1{};
        vec1.resize(5);
        expect(eq(vec1.size(), 5));
        expect(eq(vec1.capacity(), capacity(5)));
        expect(eq(vec1[0], 0));
        expect(eq(vec1[1], 0));
        expect(eq(vec1[2], 0));
        expect(eq(vec1[3], 0));
        expect(eq(vec1[4], 0));

        vec1.resize(2);
        expect(eq(vec1.size(), 2));
        expect(eq(vec1.capacity(), capacity(5)));
        expect(eq(vec1[0], 0));
        expect(eq(vec1[1], 0));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_emplace_back : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("emplace_back(Args...)") = []() {
      vector vec1{};
      expect(eq(vec1.emplace_back(0), 0));
      expect(eq(vec1.emplace_back(1), 1));
      expect(eq(vec1.emplace_back(2), 2));
      expect(eq(vec1.size(), 3));
      expect(eq(vec1[0], 0));
      expect(eq(vec1[1], 1));
      expect(eq(vec1[2], 2));

      if constexpr (sh::copy_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec2{};
        expect(eq(vec2.emplace_back(v1), 0));
        expect(eq(vec2.emplace_back(v2), 1));
        expect(eq(vec2.emplace_back(v3), 2));
        expect(eq(vec2.size(), 3));
        expect(eq(vec2, vector{0, 1, 2}));
      }

      if constexpr (sh::move_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec3{};
        vec3.emplace_back(std::move(v1));
        vec3.emplace_back(std::move(v2));
        vec3.emplace_back(std::move(v3));
        expect(eq(vec3.size(), 3));
        expect(eq(vec3[0], 0));
        expect(eq(vec3[1], 1));
        expect(eq(vec3[2], 2));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_push_back : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("push_back(const value_type&)") = []() {
      if constexpr (sh::copy_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec2{};
        vec2.push_back(v1);
        vec2.push_back(v2);
        vec2.push_back(v3);
        expect(eq(vec2.size(), 3));
        expect(eq(vec2, vector{0, 1, 2}));
      }
    };

    test("push_back(value_type&&)") = []() {
      if constexpr (sh::move_constructible<T>) {
        T v1(0);
        T v2(1);
        T v3(2);
        vector vec3{};
        vec3.push_back(std::move(v1));
        vec3.push_back(std::move(v2));
        vec3.push_back(std::move(v3));
        expect(eq(vec3.size(), 3));
        expect(eq(vec3[0], 0));
        expect(eq(vec3[1], 1));
        expect(eq(vec3[2], 2));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_pop_back : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("pop_back()") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(0);
      expect(eq(vec1.size(), 2));
      vec1.pop_back();
      vec1.pop_back();
      expect(eq(vec1.size(), 0));
    };

    test("pop_back(size_type)") = []() {
      vector vec1{};
      vec1.emplace_back(0);
      vec1.emplace_back(0);
      expect(eq(vec1.size(), 2));
      vec1.pop_back(2);
      expect(eq(vec1.size(), 0));
    };
  }
};

template <typename T, std::size_t N>
struct tests_swap : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::capacity;
  using tests<T, N>::test;

  static void run() {
    test("swap(const vector&)") = []() {
      if constexpr (N == 0 || sh::copy_constructible<T>) {
        vector vec1{};
        vec1.emplace_back(0);
        vec1.emplace_back(1);
        vec1.emplace_back(2);
        vector vec2{};
        vec2.emplace_back(0);
        vec2.emplace_back(1);
        vec2.emplace_back(2);

        vector vec3{};
        vector vec4{};
        vec3.swap(vec1);
        vec4.swap(vec2);

        expect(eq(vec1, vec2));
        expect(eq(vec3, vec4));
      }
    };
  }
};

template <typename T, std::size_t N>
struct tests_typenames : tests<T, N> {
  using typename tests<T, N>::vector;
  using tests<T, N>::test;

  static void run() {
    test("typenames") = []() {
      expect(std::is_same_v<typename vector::value_type, T>);
      expect(std::is_same_v<typename vector::size_type, std::size_t>);
      expect(std::is_same_v<typename vector::difference_type, std::make_signed_t<std::size_t>>);
      expect(std::is_same_v<typename vector::reference, T&>);
      expect(std::is_same_v<typename vector::const_reference, const T&>);
      expect(std::is_same_v<typename vector::pointer, T*>);
      expect(std::is_same_v<typename vector::const_pointer, const T*>);
      expect(std::is_same_v<typename vector::iterator, T*>);
      expect(std::is_same_v<typename vector::const_iterator, const T*>);
      expect(std::is_same_v<typename vector::reverse_iterator, std::reverse_iterator<T*>>);
      expect(
          std::is_same_v<typename vector::const_reverse_iterator, std::reverse_iterator<const T*>>);
    };
  }
};

template <template <typename, std::size_t> typename Test, typename T>
void run() {
  Test<T, 0>::run();
  Test<T, 1>::run();
  Test<T, 2>::run();
  Test<T, 4>::run();
  Test<T, 8>::run();
}

template <template <typename, std::size_t> typename Test>
void run() {
  run<Test, int>();
  run<Test, test_type1>();
  run<Test, test_type2>();
  run<Test, test_type3>();
  run<Test, test_type4>();
}

}  // namespace

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
  run<tests_typenames>();
}
