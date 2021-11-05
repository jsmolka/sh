#pragma once

#include <sh/scope_guard.h>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <memory>
#include <string>
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
      reallocate(capacity);
    }
  }

  // Todo: shrink_to_fit
#pragma endregion

#pragma region modifiers
  void clear() {
    std::destroy(begin(), end());
    head_ = data_;
  }

  template <typename... Args>
  // requires std::constructible_from<value_type, Args...>
  auto emplace(const_iterator pos, Args&&... args) -> iterator {
    const auto index = std::distance(cbegin(), pos);
    assert(index <= size());
    grow_to_fit();
    std::move_backward(begin() + index, end(), end() + 1);
    head_++;
    return std::construct_at(begin() + index, std::forward<Args>(args)...);
  }

  auto insert(const_iterator pos, const value_type& value) -> iterator {
    return emplace(pos, value);
  }

  auto insert(const_iterator pos, value_type&& value) -> iterator {
    return emplace(pos, std::move(value));
  }

  auto insert(const_iterator pos, size_type count, const value_type& value) -> iterator {
    const auto index = std::distance(cbegin(), pos);
    assert(index <= size());
    if (count == 0) {
      return pos;
    }
    grow_to_fit(count);
    auto item = std::move_backward(begin() + index, end(), end() + count) - 1;
    while (count--) {
      *item++ = value;
    }
    return begin() + index;
  }

  template <typename Iterator>
  auto insert(const_iterator pos, Iterator first, Iterator last) -> iterator {
    const auto index = std::distance(cbegin(), pos);
    const auto count = std::distance(first, last);
    assert(index <= size());
    if (count == 0) {
      return pos;
    }
    grow_to_fit(count);
    auto item = std::move_backward(begin() + index, end(), end() + count) - 1;
    while (first != last) {
      *item++ = *first++;
    }
    return begin() + index;
  }

  auto insert(const_iterator pos, std::initializer_list<value_type> values) -> iterator {
    return insert(pos, values.begin(), end());
  }

  auto erase(const_iterator pos) -> iterator {
    std::move(pos + 1, end(), pos);
    std::destroy_at(--head_);
    return pos;
  }

  auto erase(const_iterator pos, size_type count) -> iterator {
    std::move(pos + count, end(), pos);
    std::destroy(end());
    return pos;
  }

  auto erase(const_iterator first, const_iterator last) -> iterator {
    if (first == last) {
      return last;
    } else {
      return erase(first, std::distance(first, last));
    }
  }

  void resize(size_type size, const value_type& value) {
    resize_impl(size, value);
  }

  void resize(size_type size) {
    resize_impl(size);
  }

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
  using storage = std::aligned_storage_t<sizeof(value_type), alignof(value_type)>;

  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void resize_impl(size_type size, Args&&... args) {
    if (size > capacity()) {
      reallocate(size);
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
      reallocate(data_ ? 2 * capacity() : 1);
    }
  }

  void grow_to_fit(size_type count) {
    if (head_ + count > last_) {
      reallocate(capacity() + count);
    }
  }

  auto reallocate_copy(pointer dest) -> pointer {
    assert(data_ && dest);
    dest = std::copy(begin(), end(), dest);
    delete data_;
    return dest;
  }

  auto reallocate_nothrow_move_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    for (auto& value : *this) {
      std::construct_at(dest++, std::move(value));
    }
    delete data_;
    return dest;
  }

  auto reallocate_move_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    scope_guard cleanup([dest]() {
      delete dest;
    });
    dest = std::uninitialized_move(begin(), end(), dest);
    delete data_;
    cleanup.release();
    return dest;
  }

  auto reallocate_nothrow_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    for (const auto& value : *this) {
      std::construct_at(dest++, value);
    }
    delete data_;
    return dest;
  }

  auto reallocate_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    scope_guard cleanup([dest]() {
      delete dest;
    });
    dest = std::uninitialized_copy(begin(), end(), dest);
    delete data_;
    cleanup.release();
    return dest;
  }

  void reallocate(size_type capacity) {
    assert(capacity > 0);

    pointer data_new = reinterpret_cast<pointer>(new storage[capacity]);
    pointer head_new;

    if (data_) {
      if constexpr (std::is_trivially_copyable_v<value_type>) {
        head_new = reallocate_copy(data_new);
      } else if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
        head_new = reallocate_nothrow_move_construct(data_new);
      } else if constexpr (std::is_move_constructible_v<value_type>) {
        head_new = reallocate_move_construct(data_new);
      } else if constexpr (std::is_nothrow_copy_constructible_v<value_type>) {
        head_new = reallocate_nothrow_copy_construct(data_new);
      } else {
        head_new = reallocate_copy_construct(data_new);
      }
    } else {
      head_new = data_new;
    }

    data_ = data_new;
    head_ = head_new;
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
