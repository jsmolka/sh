#pragma once

#include <cassert>

#include <sh/algorithm.h>
#include <sh/concepts.h>
#include <sh/memory.h>

namespace sh {

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

  vector() noexcept = default;

  vector(size_type count, const value_type& value) requires sh::copy_constructible<value_type> {
    allocate(count);
    head_ = sh::uninitialized_fill_n(begin(), count, value);
  }

  explicit vector(size_type count) requires std::default_initializable<value_type> {
    allocate(count);
    head_ = sh::uninitialized_default_construct_n(begin(), count);
  }

  template <std::random_access_iterator I>
    requires(std::constructible_from<value_type, std::iter_reference_t<I>>)
  vector(I first, I last) {
    allocate(std::distance(first, last));
    head_ = sh::uninitialized_copy(first, last, begin());
  }

  template <std::input_iterator I, std::sentinel_for<I> S>
    requires(std::constructible_from<value_type, std::iter_reference_t<I>>)
  vector(I first, S last) {
    for (; first != last; ++first) {
      push_back(*first);
    }
  }

  vector(const vector& other) requires sh::copy_constructible<value_type>
      : vector(other.begin(), other.end()) {}

  vector(vector&& other) noexcept : data_(other.data_), head_(other.head_), last_(other.last_) {
    other.data_ = nullptr;
  }

  vector(std::initializer_list<value_type> list) requires sh::copy_constructible<value_type>
      : vector(list.begin(), list.end()) {}

  ~vector() {
    if (data_) {
      destroy();
      deallocate();
    }
  }

  auto operator=(std::initializer_list<value_type> list)
      -> vector& requires sh::copy_constructible<value_type> {
    assign(list.begin(), list.end());
    return *this;
  }

  auto operator=(const vector& other) -> vector& requires sh::copy_constructible<value_type> {
    if (this != &other) [[likely]] {
      assign(other.begin(), other.end());
    }
    return *this;
  }

  auto operator=(vector&& other) -> vector& {
    if (this != &other) [[likely]] {
      ~vector();
      swap(other);
      other.data_ = nullptr;
    }
    return *this;
  }

  void assign(size_type count,
              const value_type& value) requires sh::copy_constructible<value_type> {
    destroy();
    uninitialized_reserve(count);
    head_ = sh::uninitialized_fill_n(begin(), count, value);
  }

  template <std::random_access_iterator I, std::sentinel_for<I> S>
    requires std::constructible_from<value_type, std::iter_reference_t<I>>
  void assign(I first, S last) {
    destroy();
    uninitialized_reserve(std::distance(first, last));
    head_ = sh::uninitialized_copy(first, last, begin());
  }

  template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::constructible_from<value_type, std::iter_reference_t<I>>
  void assign(I first, S last) {
    clear();
    for (; first != last; ++first) {
      push_back(*first);
    }
  }

  void assign(std::initializer_list<value_type> list) requires sh::copy_constructible<value_type> {
    assign(list.begin(), list.end());
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
    if (data_) {
      reallocate(size());
    }
  }

  void clear() {
    destroy();
    head_ = data_;
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args...> && sh::move_constructible<value_type> &&
        sh::move_assignable<value_type>
  auto emplace(const_iterator pos, Args&&... args) -> iterator {
    if (pos == end()) {
      grow_to_fit();
      return std::construct_at(head_++, std::forward<Args>(args)...);
    } else {
      const auto index = std::distance(cbegin(), pos);
      assert(index < size());
      grow_to_fit();
      std::construct_at(end(), std::move(end()[-1]));
      std::move_backward(begin() + index, end() - 1, end());  // Todo: optimize with memmove?
      auto& item = (*this)[index];
      item = value_type(std::forward<Args>(args)...);
      head_++;
      return static_cast<iterator>(&item);
    }
  }

  auto insert(const_iterator pos, const value_type& value)
      -> iterator requires std::copy_constructible<value_type> {
    return emplace(pos, value);
  }

  auto insert(const_iterator pos, value_type&& value)
      -> iterator requires std::move_constructible<value_type> {
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

  void swap(vector& other) {
    using std::swap;
    swap(data_, other.data_);
    swap(head_, other.head_);
    swap(last_, other.last_);
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
    if (head_ == last_) [[unlikely]] {
      reallocate(data_ ? 2 * capacity() : 1);
    }
  }

  void grow_to_fit(size_type count) {
    if (head_ + count > last_) {
      reallocate(capacity() + count);
    }
  }

  void uninitialized_reserve(size_type capacity) {
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
    destroy();
    deallocate();
    return dest;
  }

  auto reallocate_move_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    try {
      dest = sh::uninitialized_move(begin(), end(), dest);
      destroy();
      deallocate();
    } catch (...) {
      delete[] reinterpret_cast<storage*>(dest);
      throw;
    }
    return dest;
  }

  auto reallocate_nothrow_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    dest = sh::uninitialized_copy(begin(), end(), dest);
    destroy();
    deallocate();
    return dest;
  }

  auto reallocate_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    try {
      dest = sh::uninitialized_copy(begin(), end(), dest);
      destroy();
      deallocate();
    } catch (...) {
      delete[] reinterpret_cast<storage*>(dest);
      throw;
    }
    return dest;
  }

  void destroy() {
    sh::destroy(begin(), end());
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

  pointer head_{};
  pointer data_{};
  pointer last_{};
};

template <typename T, std::size_t kSize>
void swap(vector<T, kSize>& a, vector<T, kSize>& b) {
  a.swap(b);
}

template <typename T, std::size_t kSizeA, std::size_t kSizeB>
auto operator==(const vector<T, kSizeA>& a, const vector<T, kSizeB>& b) -> bool {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <typename T, std::size_t kSizeA, std::size_t kSizeB>
auto operator!=(const vector<T, kSizeA>& a, const vector<T, kSizeB>& b) -> bool {
  return !(a == b);
}

}  // namespace sh
