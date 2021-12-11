#pragma once

#include <filesystem>

#include <sh/fmt.h>
#include <sh/parse.h>

namespace sh::filesystem {

using namespace std::filesystem;

}  // namespace sh::filesystem

namespace fmt {

template <>
struct formatter<sh::filesystem::path> : formatter<std::string> {
  auto format(const sh::filesystem::path& path, format_context& ctx) {
    return formatter<std::string>::format(path.string(), ctx);
  }
};

}  // namespace fmt

template <>
inline auto sh::parse(std::string_view data) -> std::optional<sh::filesystem::path> {
  return sh::filesystem::u8path(data).make_preferred();
}
