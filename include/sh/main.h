#pragma once

#include <memory>

#include <sh/windows.h>

#if SH_CC_MSVC

auto main(int argc, char* argv[]) -> int;

// clang-format off
#pragma comment(linker, "/ENTRY:wmainCRTStartup")
// clang-format on

auto wmain(int argc, wchar_t* argv[]) -> int {
  static_assert(sizeof(std::unique_ptr<char[]>) == sizeof(char*));
  static_assert(sizeof(std::unique_ptr<std::unique_ptr<char[]>[]>) == sizeof(char**));

  auto args = std::make_unique<std::unique_ptr<char[]>[]>(argc);
  for (int i = 0; i < argc; ++i) {
    auto size = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);
    auto data = std::make_unique<char[]>(std::max(size + 1, 2));
    WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, data.get(), size, NULL, NULL);
    data[size + 1] = '\0';
    args[i] = std::move(data);
  }
  return main(argc, reinterpret_cast<char**>(args.get()));
}

#endif
