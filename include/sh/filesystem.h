#pragma once

#include <filesystem>

#include <sh/fmt.h>
#include <sh/parse.h>

namespace sh {

namespace filesystem {

using namespace std::filesystem;

}  // namespace filesystem

template <>
struct parser<filesystem::path> {
  auto parse(std::string_view data) -> std::optional<filesystem::path> {
    return filesystem::u8path(data).make_preferred();
  }
};

}  // namespace sh

template <>
struct fmt::formatter<sh::filesystem::path> : fmt::formatter<std::string> {
  auto format(const sh::filesystem::path& path, fmt::format_context& ctx) {
    return fmt::formatter<std::string>::format(path.string(), ctx);
  }
};
