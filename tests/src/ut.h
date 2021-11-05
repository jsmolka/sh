#pragma once

#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>
#include <string>
#include <string_view>
#include <vector>

using namespace boost::ut;

template <typename... Args>
std::string_view make_name(const Args&... parts) {
  static std::vector<std::string> names;
  const auto name = names.emplace_back((std::string(parts) + ...));
  return std::string_view(name.begin(), name.end());
}
