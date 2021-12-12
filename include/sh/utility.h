#pragma once

#include <cassert>
#include <cstddef>

#include <sh/env.h>
#include <sh/int.h>

namespace sh {

#if SH_CC_MSVC
#  define SH_INLINE __forceinline
#  define SH_NO_INLINE __declspec(noinline)
#else
#  define SH_INLINE inline __attribute__((always_inline))
#  define SH_NO_INLINE __attribute__((noinline))
#endif

#if SH_RELEASE
#  if SH_CC_MSVC
#    define SH_UNREACHABLE __assume(0)
#  else
#    define SH_UNREACHABLE __builtin_unreachable()
#  endif
#else
#  define SH_UNREACHABLE assert(false)
#endif

template <typename... Ts>
constexpr void unused(const Ts&...) {}

template <typename Dest, typename Source>
auto cast(Source& data, std::size_t byte = 0) -> Dest& {
  return *reinterpret_cast<Dest*>(reinterpret_cast<u8*>(&data) + byte);
}

template <typename Dest, typename Source>
auto cast(const Source& data, std::size_t byte = 0) -> const Dest& {
  return *reinterpret_cast<const Dest*>(reinterpret_cast<const u8*>(&data) + byte);
}

}  // namespace sh
