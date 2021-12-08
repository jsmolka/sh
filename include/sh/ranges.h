#pragma once

#include <ranges>

#include <sh/iterator.h>

namespace sh {

template <typename Range>
concept forward_range = requires {
  typename sh::iterator_t<Range>;
  typename sh::sentinel_t<Range>;
  requires forward_iterator<sh::iterator_t<Range>>;
  requires sentinel_for<sh::sentinel_t<Range>, sh::iterator_t<Range>>;
};

template <forward_range Range>
using range_value_t = sh::iter_value_t<sh::iterator_t<Range>>;

template <forward_range Range>
using range_reference_t = sh::iter_reference_t<sh::iterator_t<Range>>;

template <typename Range>
concept reverse_range = requires {
  typename sh::reverse_iterator_t<Range>;
  typename sh::reverse_sentinel_t<Range>;
  requires forward_iterator<sh::reverse_iterator_t<Range>>;
  requires sentinel_for<sh::reverse_sentinel_t<Range>, sh::reverse_iterator_t<Range>>;
};

template <sh::forward_iterator I, sh::sentinel_for<I> S>
class range {
 public:
  range(I begin, S end) : begin_(begin), end_(end) {}

  [[nodiscard]] auto begin() -> I {
    return begin_;
  }

  [[nodiscard]] auto end() -> S {
    return end_;
  }

  [[nodiscard]] auto begin() const -> I {
    return begin_;
  }

  [[nodiscard]] auto end() const -> S {
    return end_;
  }

  [[nodiscard]] auto cbegin() const -> I {
    return begin_;
  }

  [[nodiscard]] auto cend() const -> S {
    return end_;
  }

 private:
  I begin_;
  S end_;
};

template <sh::forward_iterator I, sh::sentinel_for<I> S = std::default_sentinel_t>
class sentinel_range {
 public:
  sentinel_range(I begin) : begin_(begin) {}

  [[nodiscard]] auto begin() -> I {
    return begin_;
  }

  [[nodiscard]] auto end() -> S {
    return S{};
  }

  [[nodiscard]] auto begin() const -> I {
    return begin_;
  }

  [[nodiscard]] auto end() const -> S {
    return S{};
  }

  [[nodiscard]] auto cbegin() const -> I {
    return begin_;
  }

  [[nodiscard]] auto cend() const -> S {
    return S{};
  }

 private:
  I begin_;
};

template <forward_range Range>
auto contains(const Range& range, const range_value_t<Range>& value) {
  return std::find(std::begin(range), std::end(range), value) != std::end(range);
}

template <sh::reverse_range Range>
auto reversed(Range& range)
    -> sh::range<sh::reverse_iterator_t<Range>, sh::reverse_sentinel_t<Range>> {
  return {std::rbegin(range), std::rend(range)};
}

template <std::integral Integral, sh::forward_iterator I, sh::sentinel_for<I> S>
class enumerate_iterator {
 public:
  struct value_type {
    Integral index;
    sh::iter_reference_t<I> value;
  };

  enumerate_iterator(Integral index, I begin, S end) : index_(index), begin_(begin), end_(end) {}

  auto operator*() -> value_type {
    return {index_, *begin_};
  }

  auto operator*() const -> value_type {
    return {index_, *begin_};
  }

  auto operator++() -> enumerate_iterator& {
    ++index_;
    ++begin_;
    return *this;
  }

  auto operator!=(std::default_sentinel_t) const -> bool {
    return begin_ != end_;
  }

 private:
  Integral index_;
  I begin_;
  S end_;
};

template <sh::forward_range Range, std::integral Integral = std::size_t>
auto enumerate(Range& range, Integral start = 0) -> sh::sentinel_range<
    enumerate_iterator<Integral, sh::iterator_t<Range>, sh::sentinel_t<Range>>> {
  return {{start, std::begin(range), std::end(range)}};
}

}  // namespace sh
