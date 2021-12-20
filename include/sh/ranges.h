#pragma once

#include <ranges>

#include <sh/iterator.h>

namespace sh {

template <typename Range>
concept forward_range = requires {
  typename iterator_t<Range>;
  typename sentinel_t<Range>;
  requires forward_iterator<iterator_t<Range>>;
  requires sentinel_for<sentinel_t<Range>, iterator_t<Range>>;
};

template <forward_range Range>
using range_value_t = iter_value_t<iterator_t<Range>>;

template <forward_range Range>
using range_reference_t = iter_reference_t<iterator_t<Range>>;

template <typename Range>
concept reverse_range = requires {
  typename reverse_iterator_t<Range>;
  typename reverse_sentinel_t<Range>;
  requires forward_iterator<reverse_iterator_t<Range>>;
  requires sentinel_for<reverse_sentinel_t<Range>, reverse_iterator_t<Range>>;
};

template <reverse_range Range>
using range_reverse_value_t = iter_value_t<reverse_iterator_t<Range>>;

template <reverse_range Range>
using range_reverse_reference_t = iter_reference_t<reverse_iterator_t<Range>>;

template <forward_iterator I, sentinel_for<I> S>
class range {
public:
  range(I begin, S end)
      : begin_(begin), end_(end) {}

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

template <forward_iterator I, sentinel_for<I> S = std::default_sentinel_t>
class sentinel_range {
public:
  sentinel_range(I begin)
      : begin_(begin) {}

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
auto contains(const Range& range, const range_value_t<Range>& value) -> bool {
  return std::find(std::begin(range), std::end(range), value) != std::end(range);
}

template <reverse_range Range>
auto reversed(Range& range) -> sh::range<reverse_iterator_t<Range>, reverse_sentinel_t<Range>> {
  return {std::rbegin(range), std::rend(range)};
}

template <std::integral Integral, forward_iterator I, sentinel_for<I> S>
class enumerate_iterator {
public:
  struct value_type {
    Integral index;
    iter_reference_t<I> value;
  };

  enumerate_iterator(Integral index, I begin, S end)
      : index_(index), begin_(begin), end_(end) {}

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

template <forward_range Range, std::integral Integral = std::size_t>
auto enumerate(Range& range, Integral start = 0) 
    -> sentinel_range<enumerate_iterator<Integral, iterator_t<Range>, sentinel_t<Range>>> {
  return {{start, std::begin(range), std::end(range)}};
}

}  // namespace sh
