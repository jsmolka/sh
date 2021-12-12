#pragma once

#ifndef FMT_HEADER_ONLY
#  define FMT_HEADER_ONLY
#  define FMT_HEADER_ONLY_DEFINED
#endif

#include <sh/fmt/args.h>
#include <sh/fmt/chrono.h>
#include <sh/fmt/color.h>
#include <sh/fmt/compile.h>
#include <sh/fmt/core.h>
#include <sh/fmt/format.h>
#include <sh/fmt/os.h>
#include <sh/fmt/ostream.h>
#include <sh/fmt/printf.h>
#include <sh/fmt/ranges.h>
#include <sh/fmt/xchar.h>

#ifdef FMT_HEADER_ONLY_DEFINED
#  undef FMT_HEADER_ONLY
#  undef FMT_HEADER_ONLY_DEFINED
#endif

namespace sh {

template <typename T, typename Char = char>
concept formattable =
    fmt::is_formattable<T>::value || fmt::has_formatter<T, fmt::buffer_context<Char>>::value;

}  // namespace sh
