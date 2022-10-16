#pragma once

#include <string>

#include <sh/vector.h>

namespace sh {

inline sh::vector<std::string> args;

inline void remember(int argc, char* argv[]) {
  args.assign(argv, argv + argc);
}

}  // namespace sh
