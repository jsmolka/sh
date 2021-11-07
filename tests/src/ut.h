#pragma once

#include <string>
#include <string_view>
#include <vector>

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>
#include <sh/fmt.h>

using namespace boost::ut;

template <typename... Args>
auto make_test(std::string_view format, Args &&...args) {
  static std::vector<std::string> names;
  names.emplace_back(fmt::format(format, std::forward<Args>(args)...));
  const auto &name = names.back();
  return test(std::string_view(name.begin(), name.end()));
}
