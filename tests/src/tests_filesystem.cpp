#include "tests_filesystem.h"

#include "common.h"

namespace fs = sh::filesystem;

auto operator<<(std::ostream& out, fs::status status) -> std::ostream& {
  return out << int(status);
}

void tests_filesystem() {
  sh::test("filesystem format path") = []() {
    expect(eq(fmt::format("{}", fs::path("test")), std::string_view("test")));
  };

  sh::test("filesystem program") = []() {
    fmt::print("{}\n", fs::program());
    fmt::print("{}\n", fs::absolute("relative"));
  };

  sh::test("filesystem read/write resizable") = []() {
    std::string src = "test";
    std::string dst;

    expect(eq(fs::write("fs/text.txt", src), fs::status::ok));
    expect(eq(fs::read("fs/text.txt", dst), fs::status::ok));
    expect(eq(src, dst));
  };

  sh::test("filesystem read/write not resizable") = []() {
    std::array<char, 5> src = {0x00, 0x01, 0x02, 0x03, 0x04};
    std::array<char, 6> dst;

    expect(eq(fs::write("fs/data.bin", src), fs::status::ok));
    expect(eq(fs::read("fs/data.bin", dst), fs::status::bad_size));
  };
}
