#pragma once

#include <sh/filesystem.h>

#include "ut.h"

auto operator<<(std::ostream& out, sh::filesystem::status status) -> std::ostream& {
  return out << int(status);
}

namespace tests_filesystem {

namespace fs = sh::filesystem;

inline suite _ = [] {
  "filesystem read/write resizable"_test = [] {
    std::string src = "test";
    std::string dst;

    expect(eq(fs::write("fs/text.txt", src), fs::status::ok));
    expect(eq(fs::read("fs/text.txt", dst), fs::status::ok));
    expect(eq(src, dst));
  };

  "filesystem read/write not resizable"_test = [] {
    std::array<char, 5> src = {0x00, 0x01, 0x02, 0x03, 0x04};
    std::array<char, 6> dst;

    expect(eq(fs::write("fs/data.bin", src), fs::status::ok));
    expect(eq(fs::read("fs/data.bin", dst), fs::status::bad_size));
  };
};

}  // namespace tests_filesystem
