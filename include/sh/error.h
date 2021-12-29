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
