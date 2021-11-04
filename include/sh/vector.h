#pragma once

#include <cassert>
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace sh {

template <typename T, std::size_t kSize = 0>
class vector {
  // Use std::aligned_storage_t<sizeof(T), alignof(T)> to work with objects with no default
};

template <typename T>
class vector<T, 0> {
 public:
  using self = vector<T>;
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const reference;
  using pointer = value_type*;
  using const_pointer = const pointer;
  using iterator = pointer;
  using const_iterator = const iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

#pragma region basic
  ~vector() {
    if (data_) {
      std::destroy(begin(), end());
      std::free(data_);
    }
  }

  // Todo: ctor
  // Todo: dtor
  // Todo: op=
#pragma endregion

#pragma region element access
  auto data() -> pointer {
    return data_;
  }

  auto data() const -> const_pointer {
    return data_;
  }

  auto front() -> reference {
    assert(!empty());
    return *data_;
  }

  auto front() const -> const_reference {
    assert(!empty());
    return *data_;
  }

  auto back() -> reference {
    assert(!empty());
    return head_[-1];
  }

  auto back() const -> const_reference {
    assert(!empty());
    return head_[-1];
  }

  auto operator[](std::size_t index) -> reference {
    assert(index < size());
    return data_[index];
  }

  auto operator[](std::size_t index) const -> const_reference {
    assert(index < size());
    return data_[index];
  }
#pragma endregion

#pragma region iterators
  auto begin() -> iterator {
    return iterator(data_);
  }

  auto end() -> iterator {
    return iterator(head_);
  }

  auto begin() const -> const_iterator {
    return const_iterator(data_);
  }

  auto end() const -> const_iterator {
    return const_iterator(head_);
  }

  auto cbegin() const -> const_iterator {
    return const_iterator(data_);
  }

  auto cend() const -> const_iterator {
    return const_iterator(head_);
  }

  auto rbegin() -> reverse_iterator {
    return reverse_iterator(head_);
  }

  auto rend() -> reverse_iterator {
    return reverse_iterator(data_);
  }

  auto rbegin() const -> const_reverse_iterator {
    return const_reverse_iterator(head_);
  }

  auto rend() const -> const_reverse_iterator {
    return const_reverse_iterator(data_);
  }

  auto crbegin() const -> const_reverse_iterator {
    return const_reverse_iterator(head_);
  }

  auto crend() const -> const_reverse_iterator {
    return const_reverse_iterator(data_);
  }
#pragma endregion

#pragma region capacity
  auto empty() const -> bool {
    return head_ == data_;
  }

  auto size() const -> size_type {
    return head_ - data_;
  }

  auto capacity() const -> size_type {
    return last_ - data_;
  }

  void reserve(size_type capacity) {
    if (capacity > this->capacity()) {
      grow(capacity);
    }
  }

  // shrink_to_fit
#pragma endregion

#pragma region modifiers
  void clear() {
    std::destroy(begin(), end());
    head_ = data_;
  }

  void resize(size_type size, const value_type& value) {
    resize_impl(size, value);
  }

  void resize(size_type size) {
    resize_impl(size);
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args...> iterator emplace(const_iterator pos,
                                                                           Args&&... args) {
      return nullptr;
    }

  iterator insert(const_iterator pos, const value_type& value) {
    return nullptr;
  }

  iterator insert(const_iterator pos, value_type&& value) {
    return nullptr;
  }

  // Todo: insert
  // Todo: emplace
  // Todo: erase

  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void emplace_back(Args&&... args) {
    grow_to_fit();
    new (head_) value_type(std::forward<Args>(args)...);
    head_++;
  }

  void push_back(const value_type& value) {
    emplace_back(value);
  }

  void push_back(value_type&& value) {
    emplace_back(std::move(value));
  }

  void pop_back() {
    std::destroy_at(--head_);
  }
#pragma endregion

 private:
  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void resize_impl(size_type size, Args&&... args) {
    if (size > capacity()) {
      grow(size);
      for (; head_ != last_; ++head_) {
        new (head_) value_type(std::forward<Args>(args)...);
      }
    } else if (size < capacity()) {
      std::destroy(begin() + size, end());
      head_ -= size;
    }
  }

  void grow_to_fit() {
    if (head_ == last_) {
      grow(data_ ? 2 * capacity() : 1);
    }
  }

  void grow(size_type capacity) {
    assert(capacity > 0);

    const auto size = this->size();
    data_ = data_ ? static_cast<pointer>(std::realloc(data_, sizeof(value_type) * capacity))
                  : static_cast<pointer>(std::malloc(sizeof(value_type) * capacity));

    if (!data_) {
      throw std::bad_alloc();
    }

    head_ = data_ + size;
    last_ = data_ + capacity;
  }

  pointer head_{};
  pointer data_{};
  pointer last_{};
};

// Todo: operator==
// Todo: operator!=
// Todo: operator<=>?
// Todo: std::swap

}  // namespace sh
