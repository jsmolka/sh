#pragma once

#include <sh/argparse.h>

#include "ut.h"

namespace tests_argparse {

using namespace std::string_view_literals;

inline suite _ = [] {
  "argparse general"_test = [] {
    const char* argv[] = {"program.exe", "-a", "1", "-b=2", "-e", "5", "7", "test"};
    sh::argument_parser parser("");
    parser.add<int>("-a");
    parser.add<int>("-b");
    parser.add<int>("-c") | 3;
    parser.add<std::optional<int>>("-d");
    parser.add<std::optional<int>>("-e");
    parser.add<std::optional<int>>("-f") | 6;
    parser.add<int>("g");
    parser.add<std::string_view>("h");
    parser.add<int>("i") | 9;
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

  "argparse edge cases"_test = [] {
    const char* argv[] = {"program.exe", "-a", "-b", "-c", "-d", "e"};
    sh::argument_parser parser("");
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

  "argparse missing argument value"_test = [] {
    const char* argv[] = {"program.exe", "-x"};
    sh::argument_parser parser("program");
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "argparse missing keyword argument"_test = [] {
    const char* argv[] = {"program.exe"};
    sh::argument_parser parser("program");
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "argparse missing positional argument"_test = [] {
    const char* argv[] = {"program.exe"};
    sh::argument_parser parser("program");
    parser.add<int>("x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "argparse wrong argument value type"_test = [] {
    const char* argv[] = {"program.exe", "-x", "wrong"};
    sh::argument_parser parser("program");
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "argparse unmatched positional argument"_test = [] {
    const char* argv[] = {"program.exe", "x"};
    sh::argument_parser parser("program");
    expect(throws([&]() {
      parser.parse(std::size(argv), argv);
    }));
  };

  "argparse trim"_test = [] {
    const char* argv[] = {"program.exe", " -x ", " 1 ", " -y = 2 "};
    sh::argument_parser parser("program");
    parser.add<int>("  -x  ");
    parser.add<int>("  -y  ");
    parser.parse(std::size(argv), argv);
    expect(eq(parser.get<int>("-x"), 1));
    expect(eq(parser.get<int>("-y"), 2));
  };
};

}  // namespace tests_argparse
