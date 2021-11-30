#pragma once

#include <sh/range.h>

namespace sh {

template <std::integral Integral, typename Iterator, typename Sentinel>
  requires sh::foreach_iterators<Iterator, Sentinel>
class enumerate_iterator {
 public:
  struct value_type {
    Integral& index;
    sh::iter_reference_t<Iterator> iter;
  };

  enumerate_iterator(Integral index, Iterator begin, Sentinel end)
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
  Iterator begin_;
  Sentinel end_;
};

template <sh::foreach_range Range, std::integral Integral = std::size_t>
auto enumerate(Range& range, Integral start = 0) -> sh::sentinel_range<
    enumerate_iterator<Integral, std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>>> {
  return {{start, std::ranges::begin(range), std::ranges::end(range)}};
}

}  // namespace sh
