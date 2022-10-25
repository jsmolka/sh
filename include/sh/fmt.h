#pragma once

#ifndef FMT_HEADER_ONLY
#  define FMT_HEADER_ONLY
#  define FMT_HEADER_ONLY_DEFINED
#endif

#include <sh/deps/fmt/include/fmt/args.h>
#include <sh/deps/fmt/include/fmt/chrono.h>
#include <sh/deps/fmt/include/fmt/color.h>
#include <sh/deps/fmt/include/fmt/compile.h>
#include <sh/deps/fmt/include/fmt/core.h>
#include <sh/deps/fmt/include/fmt/format.h>
#include <sh/deps/fmt/include/fmt/os.h>
#include <sh/deps/fmt/include/fmt/ranges.h>
#include <sh/deps/fmt/include/fmt/xchar.h>

#ifdef FMT_HEADER_ONLY_DEFINED
#  undef FMT_HEADER_ONLY
#  undef FMT_HEADER_ONLY_DEFINED
#endif

namespace sh {

template<typename T, typename Char = char>
concept formattable = fmt::is_formattable<T>::value || fmt::has_formatter<T, fmt::buffer_context<Char>>::value;

}  // namespace sh
