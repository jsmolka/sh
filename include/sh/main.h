#pragma once

#include <sh/env.h>
#include <sh/vector.h>
#include <sh/windows.h>

#if SH_CC_MSVC

int main(int argc, char* argv[]);

#  pragma comment(linker, "/ENTRY:wmainCRTStartup")

int wmain(int argc, wchar_t* argv[]) {
  sh::vector<char*> args;
  for (int i = 0; i < argc; ++i) {
    const auto size = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);
    const auto data = new char[std::max(size + 1, 2)];
    WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, data, size, NULL, NULL);
    data[size + 1] = '\0';
    args.push_back(data);
  }

  const auto value = main(argc, args.data());
  for (const auto& data : args) {
    delete[] data;
  }
  return value;
}

#endif
