#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include <sh/concepts.h>

namespace sh {

namespace detail {

template <typename T, typename Derived>
class vector_base {
 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::make_signed_t<size_type>;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  vector_base() noexcept = default;

  vector_base(pointer data, pointer head, pointer last) noexcept
      : data_(data), head_(head), last_(last) {}

  ~vector_base() {
    destruct();
  }

  void assign(size_type count,
              const value_type& value) requires sh::copy_constructible<value_type> {
    std::destroy(begin(), end());
    uninitialized_reserve(count);
    head_ = std::uninitialized_fill_n(begin(), count, value);
  }

  template <std::random_access_iterator I>
    requires std::constructible_from<value_type, std::iter_reference_t<I>>
  void assign(I first, I last) {
    const auto distance = std::distance(first, last);
    assert(distance >= 0);
    std::destroy(begin(), end());
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
    return reverse_iterator(end());
  }

  [[nodiscard]] auto rend() -> reverse_iterator {
    return reverse_iterator(begin());
  }

  [[nodiscard]] auto rbegin() const -> const_reverse_iterator {
    return const_reverse_iterator(end());
  }

  [[nodiscard]] auto rend() const -> const_reverse_iterator {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] auto crbegin() const -> const_reverse_iterator {
    return const_reverse_iterator(cend());
  }

  [[nodiscard]] auto crend() const -> const_reverse_iterator {
    return const_reverse_iterator(cbegin());
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
    if (derived()->heap_allocated()) {
      reallocate(size());
    }
  }

  void clear() {
    std::destroy(begin(), end());
    head_ = data_;
  }

  template <typename... Args>
    requires sh::move_constructible<value_type> && sh::move_assignable<value_type> &&
        std::constructible_from<value_type, Args...>
  auto emplace(const_iterator pos, Args&&... args) -> iterator {
    assert(pos >= begin() && pos <= end());
    iterator where;
    if (pos == end()) {
      grow_to_fit();
      where = end();
    } else {
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
    assert(pos >= begin() && pos <= end());
    if (count == 0) [[unlikely]] {
      return const_cast<iterator>(pos);
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
    assert(pos >= begin() && pos <= end());
    if (first == last) [[unlikely]] {
      return const_cast<iterator>(pos);
    }

    const auto count = std::distance(first, last);
    const auto distance = std::distance(cbegin(), pos);
    assert(distance >= 0);
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
    assert(pos >= begin() && pos < end());
    const auto where = const_cast<iterator>(pos);
    std::move(where + 1, end(), where);
    std::destroy_at(--head_);
    return where;
  }

  auto erase(const_iterator pos, size_type count)
      -> iterator requires sh::move_assignable<value_type> {
    const auto where = const_cast<iterator>(pos);
    if (count) {
      assert(pos >= begin() && pos < end());
      assert(pos + count <= end());
      std::move(where + count, end(), where);
      std::destroy(end() - count, end());
      head_ -= count;
    }
    return where;
  }

  auto erase(const_iterator first, const_iterator last)
      -> iterator requires sh::move_assignable<value_type> {
    return erase(first, static_cast<size_type>(std::distance(first, last)));
  }

  void resize(size_type size, const value_type& value) requires sh::copy_constructible<value_type> {
    resize_impl(size, value);
  }

  void resize(size_type size) requires sh::value_constructible<value_type> {
    resize_impl(size);
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
    assert(!empty());
    std::destroy_at(--head_);
  }

  void pop_back(std::size_t count) {
    assert(count <= size());
    std::destroy(end() - count, end());
    head_ -= count;
  }

 protected:
  using storage = std::aligned_storage_t<sizeof(value_type), alignof(value_type)>;

  void construct(size_type count,
                 const value_type& value) requires sh::copy_constructible<value_type> {
    allocate(count);
    head_ = std::uninitialized_fill_n(begin(), count, value);
  }

  void construct(size_type count) requires sh::value_constructible<T> {
    allocate(count);
    head_ = std::uninitialized_value_construct_n(begin(), count);
  }

  template <std::random_access_iterator I>
    requires(std::constructible_from<value_type, std::iter_reference_t<I>>)
  void construct(I first, I last) {
    const auto distance = std::distance(first, last);
    assert(distance >= 0);
    allocate(static_cast<size_type>(distance));
    head_ = std::uninitialized_copy(first, last, begin());
  }

  void destruct() {
    std::destroy(begin(), end());
    if (derived()->heap_allocated()) {
      delete[] reinterpret_cast<storage*>(data_);
    }
  }

  pointer head_{};
  pointer data_{};
  pointer last_{};

 private:
  auto derived() -> Derived* {
    return static_cast<Derived*>(this);
  }

  template <typename... Args>
    requires std::constructible_from<value_type, Args...>
  void resize_impl(size_type size, Args&&... args) {
    if (size > this->size()) {
      if (size > capacity()) {
        reallocate(size);
      }
      if constexpr (sizeof...(Args) == 0) {
        std::uninitialized_value_construct(end(), begin() + size);
      } else {
        std::uninitialized_fill(end(), begin() + size, std::forward<Args>(args)...);
      }
    } else if (size < this->size()) {
      std::destroy(begin() + size, end());
    }
    head_ = begin() + size;
  }

  void uninitialized_reserve(size_type capacity) {
    if (capacity > this->capacity()) {
      if (derived()->heap_allocated()) {
        delete[] reinterpret_cast<storage*>(data_);
      }
      data_ = reinterpret_cast<pointer>(new storage[capacity]);
      head_ = data_;
      last_ = data_ + capacity;
    }
  }

  auto next_capacity() const -> size_type {
    return data_ ? (3 * capacity() + 1) / 2 : 1;
  }

  void grow_to_fit() {
    if (head_ == last_) [[unlikely]] {
      reallocate(next_capacity());
    }
  }

  void grow_to_fit(size_type count) {
    if (head_ + count > last_) {
      reallocate(std::max(size() + count, next_capacity()));
    }
  }

  void allocate(size_type capacity) {
    if (capacity > this->capacity()) {
      data_ = reinterpret_cast<pointer>(new storage[capacity]);
      head_ = data_;
      last_ = data_ + capacity;
    }
  }

  void reallocate(size_type capacity) {
    assert(capacity > 0);

    pointer data_new = reinterpret_cast<pointer>(new storage[capacity]);
    pointer head_new;

    if (data_) {
      try {
        if constexpr (sh::move_constructible<value_type>) {
          head_new = std::uninitialized_move(begin(), end(), data_new);
        } else {
          head_new = std::uninitialized_copy(begin(), end(), data_new);
        }
        destruct();
      } catch (...) {
        delete[] reinterpret_cast<storage*>(data_new);
        throw;
      }
    } else {
      head_new = data_new;
    }

    data_ = data_new;
    head_ = head_new;
    last_ = data_new + capacity;
  }
};

}  // namespace detail

template <typename T, std::size_t kSize = 0>
  requires sh::move_constructible<T> || sh::copy_constructible<T>
class vector : private detail::vector_base<T, vector<T, kSize>> {
 private:
  friend class detail::vector_base<T, vector<T, kSize>>;
  using base = detail::vector_base<T, vector<T, kSize>>;

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

  vector() noexcept : base(stack(), stack(), stack() + kSize) {}

  vector(size_type count, const value_type& value) requires sh::copy_constructible<value_type>
      : vector() {
    construct(count, value);
  }

  explicit vector(size_type count) requires sh::value_constructible<T> : vector() {
    construct(count);
  }

  template <std::random_access_iterator I>
    requires(std::constructible_from<value_type, std::iter_reference_t<I>>)
  vector(I first, I last) : vector() {
    construct(first, last);
  }

  vector(const vector& other) requires sh::copy_constructible<value_type>
      : vector(other.begin(), other.end()) {}

  vector(vector&& other) : vector() {
    move(std::forward<vector>(other));
  }

  vector(std::initializer_list<value_type> init) requires sh::copy_constructible<value_type>
      : vector(init.begin(), init.end()) {}

  auto operator=(const vector& other) -> vector& requires sh::copy_constructible<value_type> {
    if (this != &other) [[likely]] {
      assign(other.begin(), other.end());
    }
    return *this;
  }

  auto operator=(vector&& other) -> vector& {
    if (this != &other) [[likely]] {
      destruct();
      reset();
      move(std::forward<vector>(other));
    }
    return *this;
  }

  auto operator=(std::initializer_list<value_type> init)
      -> vector& requires sh::copy_constructible<value_type> {
    assign(init.begin(), init.end());
    return *this;
  }

  void swap(vector& other) {
    vector temp(std::move(other));
    other = std::move(*this);
    *this = std::move(temp);
  }

  using base::assign;
  using base::operator[];
  using base::front;
  using base::back;
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
  using base::emplace;
  using base::insert;
  using base::erase;
  using base::resize;
  using base::emplace_back;
  using base::push_back;
  using base::pop_back;

 protected:
  using typename base::storage;

  using base::construct;
  using base::destruct;
  using base::data_;
  using base::head_;
  using base::last_;

 private:
  auto stack() -> pointer {
    return reinterpret_cast<pointer>(stack_);
  }

  auto stack() const -> const_pointer {
    return reinterpret_cast<const_pointer>(stack_);
  }

  auto heap_allocated() const -> bool {
    return data_ && data_ != stack();
  }

  void reset() {
    data_ = stack();
    head_ = stack();
    last_ = stack() + kSize;
  }

  void move(vector&& other) {
    if (other.data_ == other.stack()) {
      if constexpr (sh::move_constructible<value_type>) {
        head_ = std::uninitialized_move(other.begin(), other.end(), begin());
      } else {
        head_ = std::uninitialized_copy(other.begin(), other.end(), begin());
      }
      std::destroy(other.begin(), other.end());
    } else {
      data_ = other.data_;
      head_ = other.head_;
      last_ = other.last_;
    }
    other.reset();
  }

  storage stack_[kSize];
};

template <typename T>
  requires sh::move_constructible<T> || sh::copy_constructible<T>
class vector<T, 0> : private detail::vector_base<T, vector<T>> {
 private:
  friend class detail::vector_base<T, vector<T>>;
  using base = detail::vector_base<T, vector<T>>;

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

  vector() noexcept = default;

  vector(size_type count, const value_type& value) requires sh::copy_constructible<value_type> {
    construct(count, value);
  }

  explicit vector(size_type count) requires sh::value_constructible<T> {
    construct(count);
  }

  template <std::random_access_iterator I>
    requires(std::constructible_from<value_type, std::iter_reference_t<I>>)
  vector(I first, I last) {
    construct(first, last);
  }

  vector(const vector& other) requires sh::copy_constructible<value_type>
      : vector(other.begin(), other.end()) {}

  vector(vector&& other) noexcept : base(other.data_, other.head_, other.last_) {
    other.reset();
  }

  vector(std::initializer_list<value_type> init) requires sh::copy_constructible<value_type>
      : vector(init.begin(), init.end()) {}

  auto operator=(const vector& other) -> vector& requires sh::copy_constructible<value_type> {
    if (this != &other) [[likely]] {
      assign(other.begin(), other.end());
    }
    return *this;
  }

  auto operator=(vector&& other) -> vector& {
    if (this != &other) [[likely]] {
      destruct();
      data_ = other.data_;
      head_ = other.head_;
      last_ = other.last_;
      other.reset();
    }
    return *this;
  }

  auto operator=(std::initializer_list<value_type> init)
      -> vector& requires sh::copy_constructible<value_type> {
    assign(init.begin(), init.end());
    return *this;
  }

  void swap(vector& other) {
    using std::swap;
    swap(data_, other.data_);
    swap(head_, other.head_);
    swap(last_, other.last_);
  }

  using base::assign;
  using base::operator[];
  using base::front;
  using base::back;
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
  using base::emplace;
  using base::insert;
  using base::erase;
  using base::resize;
  using base::emplace_back;
  using base::push_back;
  using base::pop_back;

 protected:
  using base::construct;
  using base::destruct;
  using base::data_;
  using base::head_;
  using base::last_;

 private:
  auto heap_allocated() const -> bool {
    return data_;
  }

  void reset() {
    data_ = nullptr;
    head_ = nullptr;
    last_ = nullptr;
  }
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
