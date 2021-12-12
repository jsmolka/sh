#pragma once

#include <string>
#include <string_view>
#include <vector>

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>
#include <sh/argparse.h>
#include <sh/array.h>
#include <sh/filesystem.h>
#include <sh/fmt.h>
#include <sh/hash.h>
#include <sh/int.h>
#include <sh/parse.h>
#include <sh/ranges.h>
#include <sh/stack.h>
#include <sh/utility.h>
#include <sh/vector.h>

using namespace boost::ut;
using namespace boost::ut::literals;

using sh::u8;
using sh::s8;
using sh::u16;
using sh::s16;
using sh::u32;
using sh::s32;
using sh::u64;
using sh::s64;

namespace sh {

template <typename... Args>
auto test(std::string_view format, Args&&... args) {
  static std::vector<std::string> names;
  names.emplace_back(fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
  const auto& name = names.back();
  return boost::ut::test(std::string_view(name.begin(), name.end()));
}

}  // namespace sh
