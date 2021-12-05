#pragma once

#include <ranges>

#include <sh/iterator.h>

namespace sh {

template <typename Iterator, typename Sentinel>
concept foreach_iterators = requires(Iterator& i, Sentinel& s) {
  { i != s } -> std::same_as<bool>;
  ++i;
  requires dereferencable<Iterator>;
};

template <typename Range>
concept foreach_range = requires {
  typename sh::iterator_t<Range>;
  typename sh::sentinel_t<Range>;
  requires foreach_iterators<sh::iterator_t<Range>, sh::sentinel_t<Range>>;
};

template <typename Iterator, typename Sentinel>
  requires foreach_iterators<Iterator, Sentinel>
class range {
 public:
  range(Iterator begin, Sentinel end) : begin_(begin), end_(end) {}

  [[nodiscard]] auto begin() -> Iterator {
    return begin_;
  }

  [[nodiscard]] auto end() -> Sentinel {
    return end_;
  }

  [[nodiscard]] auto begin() const -> Iterator {
    return begin_;
  }

  [[nodiscard]] auto end() const -> Sentinel {
    return end_;
  }

  [[nodiscard]] auto cbegin() const -> Iterator {
    return begin_;
  }

  [[nodiscard]] auto cend() const -> Sentinel {
    return end_;
  }

 private:
  Iterator begin_;
  Sentinel end_;
};

template <typename Iterator>
  requires foreach_iterators<Iterator, std::default_sentinel_t>
class sentinel_range {
 public:
  sentinel_range(Iterator begin) : begin_(begin) {}

  [[nodiscard]] auto begin() -> Iterator {
    return begin_;
  }

  [[nodiscard]] auto end() -> std::default_sentinel_t {
    return std::default_sentinel;
  }

  [[nodiscard]] auto begin() const -> Iterator {
    return begin_;
  }

  [[nodiscard]] auto end() const -> std::default_sentinel_t {
    return std::default_sentinel;
  }

  [[nodiscard]] auto cbegin() const -> Iterator {
    return begin_;
  }

  [[nodiscard]] auto cend() const -> std::default_sentinel_t {
    return std::default_sentinel;
  }

 private:
  Iterator begin_;
};

}  // namespace sh
