#pragma once

#include <compare>
#include <ranges>

#include <sh/iterator.h>

namespace sh {

template <typename I, typename S>
concept foreach_iterators = requires(I i, S s) {
  { i != s } -> std::same_as<bool>;
  ++i;
  requires dereferencable<I>;
};

template <typename Range>
concept foreach_range = requires {
  typename std::ranges::iterator_t<Range>;
  typename std::ranges::sentinel_t<Range>;
  requires foreach_iterators<std::ranges::iterator_t<Range>, std::ranges::sentinel_t<Range>>;
};

template <typename I, typename S>
  requires foreach_iterators<I, S>
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

  auto operator<=>(const range&) const = default;

 private:
  I begin_;
  S end_;
};

}  // namespace sh
