#pragma once

#include <string>
#include <string_view>
#include <vector>

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>
#include <sh/fmt.h>
#include <sh/int.h>

using sh::u8;
using sh::u16;
using sh::u32;
using sh::u64;
using sh::s8;
using sh::s16;
using sh::s32;
using sh::s64;

using namespace boost::ut;

template <typename... Args>
auto test(std::string_view format, Args&&... args) {
  static std::vector<std::string> names;
  names.emplace_back(fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
  const auto& name = names.back();
  return boost::ut::test(std::string_view(name.begin(), name.end()));
}
