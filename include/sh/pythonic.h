#pragma once

#include <iterator>
#include <tuple>

#include <sh/range.h>

namespace sh {

template <std::integral Integral, typename I, typename S>
class enumerate_iterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::tuple<Integral&, std::iter_reference_t<I>>;
  using reference = value_type&;
  using pointer = value_type*;

  enumerate_iterator(Integral index, I begin, S end) : index_(index), begin_(begin), end_(end) {}

  auto operator*() -> value_type {
    return std::forward_as_tuple(index_, *begin_);
  }

  auto operator*() const -> value_type {
    return std::forward_as_tuple(index_, *begin_);
  }

  auto operator++() -> enumerate_iterator& {
    ++index_;
    ++begin_;
    return *this;
  }

  auto operator==(std::default_sentinel_t) const -> bool {
    return begin_ == end_;
  }

  auto operator!=(std::default_sentinel_t) const -> bool {
    return !(*this == std::default_sentinel);
  }

  auto operator<=>(const enumerate_iterator&) const = default;

 private:
  Integral index_;
  I begin_;
  S end_;
};

template <sh::foreach_range Range, std::integral Integral = std::size_t>
auto enumerate(Range& range, Integral start = 0) -> sh::range<
    enumerate_iterator<Integral, std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>>,
    std::default_sentinel_t> {
  return {{start, std::ranges::begin(range), std::ranges::end(range)}, std::default_sentinel};
}

}  // namespace sh
