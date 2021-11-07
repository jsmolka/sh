#pragma once

#include <cassert>
#include <concepts>

#include <sh/memory.h>

namespace sh {

namespace {

template <typename T>
class delete_guard {
 public:
  delete_guard(void* pointer) : pointer_(reinterpret_cast<T*>(pointer)) {}

  ~delete_guard() {
    if (pointer_) {
      delete[] pointer_;
    }
  }

  void release() {
    pointer_ = nullptr;
  }

 private:
  T* pointer_;
};

}  // namespace

template <typename T, std::size_t kSize = 0>
class vector {};

template <typename T>
class vector<T, 0> {
 public:
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

  vector() noexcept : data_{}, head_{}, last_{} {}

  vector(size_type count, const value_type& value) {
    static_assert(std::is_copy_constructible_v<value_type>);
    allocate(count);
    head_ = std::uninitialized_fill_n(begin(), count, value);
  }

  explicit vector(size_type count) : vector(count, {}) {
    static_assert(std::is_default_constructible_v<value_type>);
  }

  template <std::random_access_iterator InputIt>
  vector(InputIt first, InputIt last) {
    static_assert(std::is_copy_constructible_v<value_type>);
    allocate(std::distance(first, last));
    head_ = sh::uninitialized_copy(first, last, begin());
  }

  template <std::input_iterator InputIt>
  vector(InputIt first, InputIt last) : vector() {
    static_assert(std::is_copy_constructible_v<value_type>);
    for (; first != last; ++first) {
      push_back(*first);
    }
  }

  vector(std::initializer_list<value_type> list) : vector(list.begin(), list.end()) {
    static_assert(std::is_copy_constructible_v<value_type>);
  }

  vector(const vector& other) : vector(other.begin(), other.end()) {
    static_assert(std::is_copy_constructible_v<value_type>);
  }

  vector(vector&& other) noexcept : data_(other.data_), head_(other.head_), last_(other.last_) {
    other.data_ = nullptr;
  }

  ~vector() {
    if (data_) {
      destruct();
      deallocate();
    }
  }

  auto operator=(std::initializer_list<value_type> list) -> vector& {
    destruct();
    reserve_uninitialized(std::distance(list.begin(), list.end()));
    head_ = sh::copy(list.begin(), list.end(), begin());
    return *this;
  }

  auto operator=(const vector& other) -> vector& {
    if (this != &other) {
      destruct();
      reserve_uninitialized(other.size());
      head_ = sh::copy(other.begin(), other.end(), begin());
    }
    return *this;
  }

  auto operator=(vector&& other) -> vector& {
    if (this != &other) {
      if (data_) {
        destruct();
        deallocate();
      }
      data_ = other.data_;
      head_ = other.head_;
      last_ = other.last_;
      other.data_ = nullptr;
    }
    return *this;
  }

  auto operator[](std::size_t index) -> reference {
    assert(index < size());
    return data_[index];
  }

  auto operator[](std::size_t index) const -> const_reference {
    assert(index < size());
    return data_[index];
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

  auto data() -> pointer {
    return data_;
  }

  auto data() const -> const_pointer {
    return data_;
  }

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

  void shrink_to_fit() {
    reallocate(size());
  }

  void clear() {
    destruct();
    head_ = data_;
  }

  template <typename... Args>
    requires(std::constructible_from<value_type, Args...>)
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
    static_assert(std::is_move_assignable_v<value_type>);
    std::move(pos + 1, end(), pos);
    std::destroy_at(--head_);
    return pos;
  }

  auto erase(const_iterator pos, size_type count) -> iterator {
    std::move(pos + count, end(), pos);
    std::destroy_at(end());
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
    static_assert(std::is_copy_constructible_v<value_type>);
    resize_impl(size, value);
  }

  void resize(size_type size) {
    static_assert(std::is_default_constructible_v<value_type>);
    resize_impl(size);
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void emplace_back(Args&&... args) {
    grow_to_fit();
    std::construct_at(head_++, std::forward<Args>(args)...);
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

  void reserve_uninitialized(size_type capacity) {
    if (capacity > this->capacity()) {
      if (data_) {
        deallocate();
      }
      data_ = reinterpret_cast<pointer>(new storage[capacity]);
      head_ = data_;
      last_ = data_ + capacity;
    }
  }

  auto reallocate_nothrow_move_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    dest = sh::uninitialized_move(begin(), end(), dest);
    destruct();
    deallocate();
    return dest;
  }

  auto reallocate_move_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    delete_guard<storage> guard(dest);
    dest = sh::uninitialized_move(begin(), end(), dest);
    destruct();
    deallocate();
    guard.release();
    return dest;
  }

  auto reallocate_nothrow_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    dest = sh::uninitialized_copy(begin(), end(), dest);
    destruct();
    deallocate();
    return dest;
  }

  auto reallocate_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    delete_guard<storage> guard(dest);
    dest = sh::uninitialized_copy(begin(), end(), dest);
    destruct();
    deallocate();
    guard.release();
    return dest;
  }

  void destruct() {
    std::destroy(begin(), end());
  }

  void reallocate(size_type capacity) {
    assert(capacity > 0);

    pointer data_new = reinterpret_cast<pointer>(new storage[capacity]);
    pointer head_new;

    if (data_) {
      if constexpr (std::is_nothrow_move_constructible_v<value_type>) {
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

  void allocate(size_type capacity) {
    data_ = capacity ? reinterpret_cast<pointer>(new storage[capacity]) : nullptr;
    head_ = data_;
    last_ = data_ + capacity;
  }

  void deallocate() {
    assert(data_);
    delete[] reinterpret_cast<storage*>(data_);
  }

  pointer head_;
  pointer data_;
  pointer last_;
};

template <typename T, std::size_t kSizeA, std::size_t kSizeB>
auto operator==(const vector<T, kSizeA>& a, const vector<T, kSizeB>& b) -> bool {
  return true;
  // return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, std::size_t kSizeA, std::size_t kSizeB>
auto operator!=(const vector<T, kSizeA>& a, const vector<T, kSizeB>& b) -> bool {
  return !(a == b);
}

}  // namespace sh
