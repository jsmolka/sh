#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <tuple>

#include <sh/env.h>
#include <sh/fmt.h>
#include <sh/parse.h>
#include <sh/windows.h>

#if SH_OS_MACOS
#  include <mach-o/dyld.h>
#elif SH_OS_BSD_FREE
#  include <sys/sysctl.h>
#  include <sys/types.h>
#endif

namespace sh::filesystem {

using namespace std::filesystem;

namespace {

template<typename T>
concept byte_pointer = std::is_pointer_v<T> && sizeof(std::remove_pointer_t<T>) == sizeof(char);

template<typename T>
concept contiguous_byte_container = requires(T&& t) {
  { t.data() } -> byte_pointer;
  { t.size() } -> std::same_as<std::size_t>;
};

template<typename T>
concept resizable = requires(T&& t) {
  t.resize(std::size_t{});
};

}  // namespace

enum class status { ok, bad_file, bad_stream, bad_size };

template<contiguous_byte_container Container>
status read(const path& file, Container& dst) {
  std::ifstream stream(file, std::ios::binary);
  if (!stream.is_open()) {
    return status::bad_file;
  } else if (!stream) {
    return status::bad_stream;
  }

  const auto size = file_size(file);
  if constexpr (resizable<Container>) {
    dst.resize(size);
  }
  if (dst.size() != size) {
    return status::bad_size;
  };

  stream.read(reinterpret_cast<char*>(dst.data()), size);
  if (!stream) {
    return status::bad_stream;
  }
  return status::ok;
}

template<contiguous_byte_container Container>
std::tuple<status, Container> read(const path& file) {
  Container data{};
  return std::make_tuple(read(file, data), data);
}

template<contiguous_byte_container Container>
status write(const path& file, const Container& src) {
  std::error_code ec;
  create_directories(file.parent_path(), ec);

  std::ofstream stream(file, std::ios::binary);
  if (!stream.is_open()) {
    return status::bad_file;
  } else if (!stream) {
    return status::bad_stream;
  }

  stream.write(reinterpret_cast<const char*>(src.data()), src.size());
  if (!stream) {
    return status::bad_stream;
  }
  return status::ok;
}

inline path program() {
#if SH_OS_WINDOWS
  WCHAR buffer[MAX_PATH];
  GetModuleFileNameW(NULL, buffer, MAX_PATH);
  return path(buffer);
#elif SH_OS_MACOS
  uint32_t size = 0;
  _NSGetExecutablePath(NULL, &size);
  auto buffer = std::make_unique<char[]>(size);
  _NSGetExecutablePath(buffer.get(), &size);
  return path(buffer.get());
#elif SH_OS_BSD_FREE
  int mib[4];
  mib[0] = CTL_KERN;
  mib[1] = KERN_PROC;
  mib[2] = KERN_PROC_PATHNAME;
  mib[3] = -1;
  char buffer[1024];
  size_t cb = sizeof(buffer);
  sysctl(mib, 4, buffer, &cb, NULL, 0);
  return path(buffer);
#elif SH_OS_BSD_NET
  return read_symlink("/proc/curproc/exe");
#elif SH_OS_BSD_DRAGONFLY
  return read_symlink("/proc/curproc/file");
#elif SH_OS_LINUX || SH_OS_UNIX
  return read_symlink("/proc/self/exe");
#else
#  error Unsupported OS
#endif
}

inline path absolute(const path& path) {
  return path.is_relative() ? program().parent_path() / path : path;
}

inline path absolute(const path& path, std::error_code&) {
  return filesystem::absolute(path);
}

}  // namespace sh::filesystem

template<>
struct sh::parser<sh::filesystem::path> {
  auto parse(std::string_view data) -> std::optional<sh::filesystem::path> {
    return sh::filesystem::u8path(data).make_preferred();
  }
};

template<>
struct fmt::formatter<sh::filesystem::path> : fmt::formatter<std::string> {
  auto format(const sh::filesystem::path& path, fmt::format_context& ctx) {
    return fmt::formatter<std::string>::format(path.string(), ctx);
  }
};
