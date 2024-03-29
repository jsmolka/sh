#pragma once

#include <sh/vector.h>

namespace sh {

template<typename T, std::size_t kSize = 0>
  requires copy_constructible<T> || move_constructible<T>
class stack : private vector<T, kSize> {
private:
  using base = vector<T, kSize>;

public:
  using typename base::value_type;
  using typename base::size_type;
  using typename base::difference_type;
  using typename base::reference;
  using typename base::const_reference;
  using typename base::pointer;
  using typename base::const_pointer;
  using typename base::iterator;
  using typename base::const_iterator;
  using typename base::reverse_iterator;
  using typename base::const_reverse_iterator;

  using base::base;
  using base::operator=;
  using base::assign;
  using base::operator[];
  using base::data;
  using base::begin;
  using base::end;
  using base::cbegin;
  using base::cend;
  using base::rbegin;
  using base::rend;
  using base::crbegin;
  using base::crend;
  using base::empty;
  using base::size;
  using base::capacity;
  using base::reserve;
  using base::shrink_to_fit;
  using base::clear;
  using base::swap;

  [[nodiscard]] auto top() -> reference {
    return this->back();
  }

  [[nodiscard]] auto top() const -> const_reference {
    return this->back();
  }

  [[nodiscard]] auto peek(std::size_t index) -> reference {
    assert(index < size());
    return end()[-index - 1];
  }

  [[nodiscard]] auto peek(std::size_t index) const -> const_reference {
    assert(index < size());
    return end()[-index - 1];
  }

  void push(const value_type& value)
      requires copy_constructible<value_type> {
    this->push_back(value);
  }

  void push(value_type&& value)
      requires move_constructible<value_type> {
    this->push_back(std::forward<value_type>(value));
  }

  template<typename... Args>
    requires std::constructible_from<value_type, Args...>
  auto emplace(Args&&... args) -> reference {
    return this->emplace_back(std::forward<Args>(args)...);
  }

  void pop() {
    this->pop_back();
  }

  void pop(std::size_t count) {
    this->pop_back(count);
  }

  auto pop_value() -> value_type
      requires move_constructible<value_type> {
    return this->pop_back_value();
  }
};

template<typename T, std::size_t kSize>
void swap(stack<T, kSize>& a, stack<T, kSize>& b) {
  a.swap(b);
}

template<typename T, std::size_t kSizeA, std::size_t kSizeB>
auto operator==(const stack<T, kSizeA>& a, const stack<T, kSizeB>& b) -> bool {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template<typename T, std::size_t kSizeA, std::size_t kSizeB>
auto operator!=(const stack<T, kSizeA>& a, const stack<T, kSizeB>& b) -> bool {
  return !(a == b);
}

}  // namespace sh
