#pragma once

#include <string>
#include <string_view>
#include <vector>

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>
#include <sh/fmt.h>

using namespace boost::ut;

template<typename... Args>
auto testf(std::string_view format, Args&&... args) {
  static std::vector<std::string> names;
  const auto& name = names.emplace_back(fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
  return test(std::string_view(name.begin(), name.end()));
}
