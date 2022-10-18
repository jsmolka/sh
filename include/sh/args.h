#pragma once

#include <string>
#include <vector>

namespace sh {

inline std::vector<std::string> args;

inline void remember(int argc, char* argv[]) {
  args.assign(argv, argv + argc);
}

}  // namespace sh
