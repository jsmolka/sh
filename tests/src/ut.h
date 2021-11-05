#pragma once

#include <string>
#include <string_view>
#include <vector>

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>
#include <sh/fmt.h>

using namespace boost::ut;

template <typename... Args>
auto make_name(std::string_view format, Args&&... args) -> std::string_view {
  static std::vector<std::string> names;
  const auto& name = names.emplace_back(fmt::format(format, std::forward<Args>(args)...));
  return std::string_view(name.begin(), name.end());
}
