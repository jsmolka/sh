#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include <sh/concepts.h>

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
    head_ = std::uninitialized_fill_n(begin(), count, value);
  }

  explicit vector(size_type count) requires sh::value_constructible<T> {
    allocate(count);
    head_ = std::uninitialized_value_construct_n(begin(), count);
  }

  template <std::random_access_iterator I>
    requires(std::constructible_from<value_type, std::iter_reference_t<I>>)
  vector(I first, I last) {
    const auto distance = std::distance(first, last);
    assert(distance >= 0);
    allocate(static_cast<size_type>(distance));
    head_ = std::uninitialized_copy(first, last, begin());
  }

  vector(const vector& other) requires sh::copy_constructible<value_type>
      : vector(other.begin(), other.end()) {}

  vector(vector&& other) noexcept : data_(other.data_), head_(other.head_), last_(other.last_) {
    other.data_ = nullptr;
  }

  vector(std::initializer_list<value_type> init) requires sh::copy_constructible<value_type>
      : vector(init.begin(), init.end()) {}

  ~vector() {
    if (data_) {
      destroy();
      deallocate();
    }
  }

  auto operator=(std::initializer_list<value_type> init)
      -> vector& requires sh::copy_constructible<value_type> {
    assign(init.begin(), init.end());
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
      this->~vector();
      swap(other);
      other.data_ = nullptr;
    }
    return *this;
  }

  void assign(size_type count,
              const value_type& value) requires sh::copy_constructible<value_type> {
    destroy();
    uninitialized_reserve(count);
    head_ = std::uninitialized_fill_n(begin(), count, value);
  }

  template <std::random_access_iterator I>
    requires std::constructible_from<value_type, std::iter_reference_t<I>>
  void assign(I first, I last) {
    assert(!inside_this(first));
    assert(!inside_this_inclusive(last));
    destroy();
    const difference_type distance = std::distance(first, last);
    assert(distance >= 0);
    uninitialized_reserve(static_cast<size_type>(distance));
    head_ = std::uninitialized_copy(first, last, begin());
  }

  void assign(std::initializer_list<value_type> init) requires sh::copy_constructible<value_type> {
    assign(init.begin(), init.end());
  }

  [[nodiscard]] auto operator[](std::size_t index) -> reference {
    assert(index < size());
    return data_[index];
  }

  [[nodiscard]] auto operator[](std::size_t index) const -> const_reference {
    assert(index < size());
    return data_[index];
  }

  [[nodiscard]] auto front() -> reference {
    assert(!empty());
    return *data_;
  }

  [[nodiscard]] auto front() const -> const_reference {
    assert(!empty());
    return *data_;
  }

  [[nodiscard]] auto back() -> reference {
    assert(!empty());
    return head_[-1];
  }

  [[nodiscard]] auto back() const -> const_reference {
    assert(!empty());
    return head_[-1];
  }

  [[nodiscard]] auto data() -> pointer {
    return data_;
  }

  [[nodiscard]] auto data() const -> const_pointer {
    return data_;
  }

  [[nodiscard]] auto begin() -> iterator {
    return iterator(data_);
  }

  [[nodiscard]] auto end() -> iterator {
    return iterator(head_);
  }

  [[nodiscard]] auto begin() const -> const_iterator {
    return const_iterator(data_);
  }

  [[nodiscard]] auto end() const -> const_iterator {
    return const_iterator(head_);
  }

  [[nodiscard]] auto cbegin() const -> const_iterator {
    return const_iterator(data_);
  }

  [[nodiscard]] auto cend() const -> const_iterator {
    return const_iterator(head_);
  }

  [[nodiscard]] auto rbegin() -> reverse_iterator {
    return std::make_reverse_iterator(end());
  }

  [[nodiscard]] auto rend() -> reverse_iterator {
    return std::make_reverse_iterator(begin());
  }

  [[nodiscard]] auto rbegin() const /* -> const_reverse_iterator */ {
    return std::make_reverse_iterator(end());
  }

  [[nodiscard]] auto rend() const /* -> const_reverse_iterator */ {
    return std::make_reverse_iterator(begin());
  }

  [[nodiscard]] auto crbegin() const /* -> const_reverse_iterator */ {
    return std::make_reverse_iterator(cend());
  }

  [[nodiscard]] auto crend() const /* -> const_reverse_iterator */ {
    return std::make_reverse_iterator(cbegin());
  }

  [[nodiscard]] auto empty() const -> bool {
    return head_ == data_;
  }

  [[nodiscard]] auto size() const -> size_type {
    return head_ - data_;
  }

  [[nodiscard]] auto capacity() const -> size_type {
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
    requires sh::move_constructible<value_type> && sh::move_assignable<value_type> &&
        std::constructible_from<value_type, Args...>
  auto emplace(const_iterator pos, Args&&... args) -> iterator {
    pointer where;
    if (pos == end()) {
      grow_to_fit();
      where = end();
    } else {
      assert(inside_this(pos));
      const auto distance = std::distance(cbegin(), pos);
      grow_to_fit();
      where = begin() + distance;
      std::construct_at(end(), std::move(end()[-1]));
      std::move_backward(where, end() - 1, end());
      std::destroy_at(where);
    }
    head_++;
    return std::construct_at(where, std::forward<Args>(args)...);
  }

  auto insert(const_iterator pos, const value_type& value)
      -> iterator requires sh::move_constructible<value_type> && sh::move_assignable<value_type> &&
      std::copy_constructible<value_type> {
    return emplace(pos, value);
  }

  auto insert(const_iterator pos, value_type&& value)
      -> iterator requires std::move_constructible<value_type> && sh::move_assignable<value_type> {
    return emplace(pos, std::move(value));
  }

  auto insert(const_iterator pos, size_type count, const value_type& value)
      -> iterator requires std::move_constructible<value_type> && sh::move_assignable<value_type> &&
      sh::copy_constructible<value_type> {
    assert(inside_this_inclusive(pos));
    if (count == 0) [[unlikely]] {
      return pos;
    }

    const auto distance = std::distance(cbegin(), pos);
    grow_to_fit(count);
    const auto where = begin() + distance;
    const auto where_end = where + count;

    if (where == end()) {                                    // 1 1 1 1 1 - - -
      std::uninitialized_fill(where, where_end, value);      // 1 1 1 1 1 2 2 2
    } else if (size() > count) {                             // 1 1 1 1 1 - - -
      std::uninitialized_move(end() - count, end(), end());  // 1 1 0 0 0 1 1 1
      std::move(where, end() - count, where_end);            // 1 0 0 0 1 1 1 1
      std::fill(where, where_end, value);                    // 1 2 2 2 1 1 1 1
    } else {                                                 // 1 1 1 - - - - -
      std::uninitialized_move(where, end(), where_end);      // 1 0 0 - - - 1 1
      std::fill(where, end(), value);                        // 1 2 2 - - - 1 1
      std::uninitialized_fill(end(), where_end, value);      // 1 2 2 2 2 2 1 1
    }
    head_ += count;
    return where;
  }

  template <std::random_access_iterator I>
  auto insert(const_iterator pos, I first, I last)
      -> iterator requires std::move_constructible<value_type> && sh::move_assignable<value_type> &&
      sh::copy_constructible<value_type> {
    assert(inside_this_inclusive(pos));
    assert(!inside_this(first));
    assert(!inside_this_inclusive(last));
    if (first == last) [[unlikely]] {
      return pos;
    }

    const auto count = std::distance(first, last);
    const auto distance = std::distance(cbegin(), pos);
    grow_to_fit(count);
    const auto where = begin() + distance;
    const auto where_end = where + count;

    if (where == end()) {                                                       // 1 1 1 1 1 - - -
      std::uninitialized_copy(first, last, where);                              // 1 1 1 1 1 2 2 2
    } else if (size() > count) {                                                // 1 1 1 1 1 - - -
      std::uninitialized_move(end() - count, end(), end());                     // 1 1 0 0 0 1 1 1
      std::move(where, end() - count, where_end);                               // 1 0 0 0 1 1 1 1
      std::copy(first, last, where);                                            // 1 2 2 2 1 1 1 1
    } else {                                                                    // 1 1 1 1 1 - - -
      std::uninitialized_move(where, end(), where_end);                         // 1 0 0 - - - 1 1
      const auto uninitialized = first + std::distance(where, end());           // 1 0 0 ^ - - 1 1
      const auto uninitialized_where = std::copy(first, uninitialized, where);  // 1 2 2 - - - 1 1
      std::uninitialized_copy(uninitialized, last, uninitialized_where);        // 1 2 2 2 2 2 1 1
    }
    head_ += count;
    return where;
  }

  auto insert(const_iterator pos, std::initializer_list<value_type> init)
      -> iterator requires std::move_constructible<value_type> && sh::move_assignable<value_type> &&
      sh::copy_constructible<value_type> {
    return insert(pos, init.begin(), init.end());
  }

  auto erase(const_iterator pos) -> iterator requires sh::move_assignable<value_type> {
    assert(inside_this(pos));
    std::move(pos + 1, end(), pos);
    std::destroy_at(--head_);
    return pos;
  }

  auto erase(const_iterator pos, size_type count)
      -> iterator requires sh::move_assignable<value_type> {
    // assert(inside_this(pos));
    // assert(inside_this(pos + count));
    if (count) {
      std::move(pos + count, end(), pos);
      std::destroy(end() - count, end());
      head_ -= count;
    }
    return pos;
  }

  auto erase(const_iterator first, const_iterator last)
      -> iterator requires sh::move_assignable<value_type> {
    return erase(first, std::distance(first, last));
  }

  void resize(size_type size, const value_type& value) requires sh::copy_constructible<value_type> {
    do_resize(size, value);
  }

  void resize(size_type size) requires std::default_initializable<value_type> {
    do_resize(size);
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void emplace_back(Args&&... args) {
    grow_to_fit();
    std::construct_at(head_++, std::forward<Args>(args)...);
  }

  void push_back(const value_type& value) requires sh::copy_constructible<value_type> {
    emplace_back(value);
  }

  void push_back(value_type&& value) requires sh::move_constructible<value_type> {
    emplace_back(std::forward<value_type>(value));
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

  template <typename Iterator>
  auto inside_this(Iterator it) const -> bool {
    if constexpr (std::same_as<iterator, std::remove_const_t<Iterator>>) {
      return it >= begin() && it < end();
    } else if constexpr (std::same_as<reverse_iterator, std::remove_const_t<Iterator>>) {
      return inside_this(it.base());
    } else {
      return false;
    }
  }

  template <typename Iterator>
  auto inside_this_inclusive(Iterator it) const -> bool {
    if constexpr (std::same_as<iterator, std::remove_const_t<Iterator>>) {
      return it >= begin() && it <= end();
    } else if constexpr (std::same_as<reverse_iterator, std::remove_const_t<Iterator>>) {
      return inside_this(it.base());
    } else {
      return false;
    }
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void do_resize(size_type size, Args&&... args) {
    if (size > capacity()) {
      reallocate(size);
      for (; head_ != last_; ++head_) {
        std::construct_at(head_, std::forward<Args>(args)...);
      }
    } else if (size < capacity()) {
      std::destroy(begin() + size, end());
      head_ = data_ + size;
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
    dest = std::uninitialized_move(begin(), end(), dest);
    destroy();
    deallocate();
    return dest;
  }

  auto reallocate_move_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    try {
      dest = std::uninitialized_move(begin(), end(), dest);
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
    dest = std::uninitialized_copy(begin(), end(), dest);
    destroy();
    deallocate();
    return dest;
  }

  auto reallocate_copy_construct(pointer dest) -> pointer {
    assert(data_ && dest);
    try {
      dest = std::uninitialized_copy(begin(), end(), dest);
      destroy();
      deallocate();
    } catch (...) {
      delete[] reinterpret_cast<storage*>(dest);
      throw;
    }
    return dest;
  }

  void destroy() {
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
