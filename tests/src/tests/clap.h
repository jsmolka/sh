#pragma once

#include <sh/clap.h>

#include "ut.h"

namespace tests_clap {

using namespace std::string_view_literals;

inline suite _ = [] {
  "clap general"_test = [] {
    const char* argv[] = {"program.exe", "-a", "1", "-b=2", "-e", "5", "7", "test"};
    sh::clap parser("");
    parser.add<int>("-a");
    parser.add<int>("-b");
    parser.add<int>("-c") << 3;
    parser.add<std::optional<int>>("-d");
    parser.add<std::optional<int>>("-e");
    parser.add<std::optional<int>>("-f") << 6;
    parser.add<int>("g");
    parser.add<std::string_view>("h");
    parser.add<int>("i") << 9;
    parser.parse(std::size(argv), argv);
    expect(eq(parser.get<int>("-a"), 1));
    expect(eq(parser.get<int>("-b"), 2));
    expect(eq(parser.get<int>("-c"), 3));
    expect(eq(parser.get<std::optional<int>>("-d").has_value(), false));
    expect(eq(parser.get<std::optional<int>>("-e").value(), 5));
    expect(eq(parser.get<std::optional<int>>("-f").value(), 6));
    expect(eq(parser.get<int>("g"), 7));
    expect(eq(parser.get<std::string_view>("h"), "test"sv));
    expect(eq(parser.get<int>("i"), 9));
  };

  "clap edge cases"_test = [] {
    const char* argv[] = {"program.exe", "-a", "-b", "-c", "-d", "e"};
    sh::clap parser("");
    parser.add<bool>("-a");
    parser.add<bool>("-b");
    parser.add<std::string_view>("-c");
    parser.add<std::optional<std::string_view>>("-d");
    parser.add<std::string_view>("e");
    parser.parse(std::size(argv), argv);
    expect(eq(parser.get<bool>("-a"), true));
    expect(eq(parser.get<bool>("-b"), true));
    expect(eq(parser.get<std::string_view>("-c"), "-d"sv));
    expect(eq(parser.get<std::optional<std::string_view>>("-d").has_value(), false));
    expect(eq(parser.get<std::string_view>("e"), "e"sv));
  };

  "clap missing argument value"_test = [] {
    const char* argv[] = {"program.exe", "-x"};
    sh::clap parser("program");
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "clap force positional"_test = [] {
    const char* argv[] = {"program.exe", "--", "-x", "--"};
    sh::clap parser("program");
    parser.add<int>("-x") << 0;
    parser.add<std::string_view>("y");
    parser.add<std::string_view>("z");
    parser.parse(std::size(argv), argv);
    expect(eq(parser.get<std::string_view>("y"), "-x"sv));
    expect(eq(parser.get<std::string_view>("z"), "--"sv));
  };

  "clap missing keyword argument"_test = [] {
    const char* argv[] = {"program.exe"};
    sh::clap parser("program");
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "clap missing positional argument"_test = [] {
    const char* argv[] = {"program.exe"};
    sh::clap parser("program");
    parser.add<int>("x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "clap wrong argument value type"_test = [] {
    const char* argv[] = {"program.exe", "-x", "wrong"};
    sh::clap parser("program");
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "clap unmatched positional argument"_test = [] {
    const char* argv[] = {"program.exe", "x"};
    sh::clap parser("program");
    parser.parse(std::size(argv), argv);
    expect(eq(parser.unmatched[0], std::string("x")));
  };

  "clap trim"_test = [] {
    const char* argv[] = {"program.exe", " -x ", " 1 ", " -y = 2 "};
    sh::clap parser("program");
    parser.add<int>("  -x  ");
    parser.add<int>("  -y  ");
    parser.parse(std::size(argv), argv);
    expect(eq(parser.get<int>("-x"), 1));
    expect(eq(parser.get<int>("-y"), 2));
  };

  "clap pointer"_test = [] {
    const char* argv[] = {"program.exe", "-a", "1", "-e=5"};
    int a;
    int b;
    int c;
    std::optional<int> d;
    std::optional<int> e;
    sh::clap parser("program");
    parser.add<decltype(a)>("-a") << &a;
    parser.add<decltype(b)>("-b") << &b << 2;
    parser.add<decltype(c)>("-c") << 3 << &c;
    parser.add<decltype(d)>("-d") << 4 << &d;
    parser.add<decltype(e)>("-e") << 5 << &e;
    parser.parse(std::size(argv), argv);
    expect(eq(a, 1));
    expect(eq(b, 2));
    expect(eq(c, 3));
    expect(eq(*d, 4));
    expect(eq(*e, 5));
  };

  "clap event"_test = [] {
    const char* argv[] = {"program.exe", "-a", "1"};
    sh::clap parser("program");
    int a = 0;
    int b = 0;
    parser.add<int>("-a") << [&](int value) { a = value; } << [&](int value) { b = value; };
    parser.parse(std::size(argv), argv);
    expect(eq(a, 1));
    expect(eq(b, 1));
  };
};

}  // namespace tests_clap
