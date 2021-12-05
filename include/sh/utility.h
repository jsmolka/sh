#pragma once

#include <cassert>

#include <sh/env.h>

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

}  // namespace sh
