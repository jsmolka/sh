#pragma once

#include <stdexcept>
#include <sh/fmt.h>

namespace sh {

class error : public std::runtime_error {
public:
  template<typename... Args>
  error(std::string_view format, Args&&... args)
    : runtime_error(fmt::format(fmt::runtime(format), std::forward<Args>(args)...)) {}
};

}  // namespace sh

template<>
struct fmt::formatter<sh::error> : fmt::formatter<const char*> {
  auto format(const sh::error& error, fmt::format_context& ctx) {
    return fmt::formatter<const char*>::format(error.what(), ctx);
  }
};
