#pragma once

#include <sh/int.h>
#include <sh/iterator.h>

namespace sh {

inline auto murmur(const void* buffer, u64 size, u64 seed) -> u64 {
  constexpr auto kM = 0xC6A4'A793'5BD1'E995;
  constexpr auto kR = 47;

  auto hash = seed ^ (size * kM);
  auto data = reinterpret_cast<const u64*>(buffer);
  auto last = data + size / 8;

  while (data != last) {
    auto k = *data++;

    k *= kM;
    k ^= k >> kR;
    k *= kM;

    hash ^= k;
    hash *= kM;
  }

  // clang-format off
  auto remaining = reinterpret_cast<const u8*>(data);
  switch (size & 0x7) {
    case 7: hash ^= static_cast<u64>(remaining[6]) << 48; [[fallthrough]];
    case 6: hash ^= static_cast<u64>(remaining[5]) << 40; [[fallthrough]];
    case 5: hash ^= static_cast<u64>(remaining[4]) << 32; [[fallthrough]];
    case 4: hash ^= static_cast<u64>(remaining[3]) << 24; [[fallthrough]];
    case 3: hash ^= static_cast<u64>(remaining[2]) << 16; [[fallthrough]];
    case 2: hash ^= static_cast<u64>(remaining[1]) <<  8; [[fallthrough]];
    case 1: hash ^= static_cast<u64>(remaining[0]) <<  0;
            hash *= kM;
  }
  // clang-format on

  hash ^= hash >> kR;
  hash *= kM;
  hash ^= hash >> kR;

  return hash;
}

template <typename T>
auto hash(const T& value) -> u64 {
  return murmur(&value, sizeof(T), 0);
}

template <sh::forward_iterator I, sh::sentinel_for<I> S>
auto hash(I first, S last) -> u64 {
  u64 seed = 0;
  while (first != last) {
    const auto& value = *first++;
    seed = murmur(&value, sizeof(value), seed);
  }
  return seed;
}

}  // namespace sh
