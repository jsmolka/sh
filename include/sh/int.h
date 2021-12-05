#pragma once

#include <cstddef>
#include <cstdint>

namespace sh {

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

// clang-format off
template <std::size_t kBytes>
struct signed_int {};
template <> struct signed_int<1> { using type =  s8; };
template <> struct signed_int<2> { using type = s16; };
template <> struct signed_int<4> { using type = s32; };
template <> struct signed_int<8> { using type = s64; };
// clang-format on

template <std::size_t kBytes>
using int_t = typename signed_int<kBytes>::type;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// clang-format off
template <std::size_t kBytes>
struct unsigned_int {};
template <> struct unsigned_int<1> { using type =  u8; };
template <> struct unsigned_int<2> { using type = u16; };
template <> struct unsigned_int<4> { using type = u32; };
template <> struct unsigned_int<8> { using type = u64; };
// clang-format on

template <std::size_t kBytes>
using uint_t = typename unsigned_int<kBytes>::type;

}  // namespace sh
