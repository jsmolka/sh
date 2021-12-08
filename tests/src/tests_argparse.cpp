#include "tests_argparse.h"

#include "common.h"

namespace {

template <typename T, std::size_t kSize>
std::size_t argc(T (&)[kSize]) {
  return kSize;
}

void tests_bool() {
  sh::test("argparse<bool>") = []() {
    const char* argv[] = {"program.exe", "-a", "-b=true", "-c=false", "true", "false"};
    sh::argument_parser parser;
    parser.add<bool>("-a");
    parser.add<bool>("-b");
    parser.add<bool>("-c");
    parser.add<bool>("d");
    parser.add<bool>("e");
    parser.add<bool>("-f") | true;
    parser.add<bool>("-g") | false;
    parser.add<bool>("h") | true;
    parser.add<bool>("i") | false;
    parser.parse(argc(argv), argv);
    expect(eq(parser.get<bool>("-a"), true));
    expect(eq(parser.get<bool>("-b"), true));
    expect(eq(parser.get<bool>("-c"), false));
    expect(eq(parser.get<bool>("d"), true));
    expect(eq(parser.get<bool>("e"), false));
    expect(eq(parser.get<bool>("-f"), true));
    expect(eq(parser.get<bool>("-g"), false));
    expect(eq(parser.get<bool>("h"), true));
    expect(eq(parser.get<bool>("i"), false));
  };
}

void tests_int() {
  sh::test("argparse<int>") = []() {
    const char* argv[] = {"program.exe", "-a", "0", "-b", "1", "-c=0b10", "0x3", "4"};
    sh::argument_parser parser;
    parser.add<int>("-a");
    parser.add<int>("-b");
    parser.add<int>("-c");
    parser.add<int>("d");
    parser.add<int>("e");
    parser.add<int>("-f") | 5;
    parser.add<int>("-g") | 6;
    parser.add<int>("h") | 7;
    parser.add<int>("i") | 8;
    parser.parse(argc(argv), argv);
    expect(eq(parser.get<int>("-a"), 0));
    expect(eq(parser.get<int>("-b"), 1));
    expect(eq(parser.get<int>("-c"), 0b10));
    expect(eq(parser.get<int>("d"), 0x3));
    expect(eq(parser.get<int>("e"), 4));
    expect(eq(parser.get<int>("-f"), 5));
    expect(eq(parser.get<int>("-g"), 6));
    expect(eq(parser.get<int>("h"), 7));
    expect(eq(parser.get<int>("i"), 8));
  };
}

void tests_double() {
  sh::test("argparse<double>") = []() {
    const char* argv[] = {"program.exe", "-a", "0.0", "-b", "0.1", "-c=0.2", "0.3", "0.4"};
    sh::argument_parser parser;
    parser.add<double>("-a");
    parser.add<double>("-b");
    parser.add<double>("-c");
    parser.add<double>("d");
    parser.add<double>("e");
    parser.add<double>("-f") | 0.5;
    parser.add<double>("-g") | 0.6;
    parser.add<double>("h") | 0.7;
    parser.add<double>("i") | 0.8;
    parser.parse(argc(argv), argv);
    expect(eq(_d(parser.get<double>("-a"), 0.001), _d(0.0)));
    expect(eq(_d(parser.get<double>("-b"), 0.001), _d(0.1)));
    expect(eq(_d(parser.get<double>("-c"), 0.001), _d(0.2)));
    expect(eq(_d(parser.get<double>("d"), 0.001), _d(0.3)));
    expect(eq(_d(parser.get<double>("e"), 0.001), _d(0.4)));
    expect(eq(_d(parser.get<double>("-f"), 0.001), _d(0.5)));
    expect(eq(_d(parser.get<double>("-g"), 0.001), _d(0.6)));
    expect(eq(_d(parser.get<double>("h"), 0.001), _d(0.7)));
    expect(eq(_d(parser.get<double>("i"), 0.001), _d(0.8)));
  };
}

template <typename String>
void tests_string() {
  sh::test("argparse<{}>", typeid(String).name()) = []() {
    const char* argv[] = {"program.exe", "-a",       "test0", "-b",
                          "test1",       "-c=test2", "test3", "test4"};
    sh::argument_parser parser;
    parser.add<String>("-a");
    parser.add<String>("-b");
    parser.add<String>("-c");
    parser.add<String>("d");
    parser.add<String>("e");
    parser.add<String>("-f") | "test5";
    parser.add<String>("-g") | "test6";
    parser.add<String>("h") | "test7";
    parser.add<String>("i") | "test8";
    parser.parse(argc(argv), argv);
    expect(eq(parser.get<String>("-a"), String("test0")));
    expect(eq(parser.get<String>("-b"), String("test1")));
    expect(eq(parser.get<String>("-c"), String("test2")));
    expect(eq(parser.get<String>("d"), String("test3")));
    expect(eq(parser.get<String>("e"), String("test4")));
    expect(eq(parser.get<String>("-f"), String("test5")));
    expect(eq(parser.get<String>("-g"), String("test6")));
    expect(eq(parser.get<String>("h"), String("test7")));
    expect(eq(parser.get<String>("i"), String("test8")));
  };
}

void tests_error() {
  sh::test("argparse missing argument value") = []() {
    const char* argv[] = {"program.exe", "-x"};
    sh::argument_parser parser;
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(argc(argv), argv);
    }));
  };

  sh::test("argparse wrong type argument") = []() {
    const char* argv[] = {"program.exe", "-x", "wrong"};
    sh::argument_parser parser;
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(argc(argv), argv);
    }));
  };

  sh::test("argparse missing keyword argument") = []() {
    const char* argv[] = {"program.exe"};
    sh::argument_parser parser;
    parser.add<int>("-x");
    expect(throws([&]() {
      parser.parse(argc(argv), argv);
    }));
  };

  sh::test("argparse missing positional argument") = []() {
    const char* argv[] = {"program.exe"};
    sh::argument_parser parser;
    parser.add<int>("x");
    expect(throws([&]() {
      parser.parse(argc(argv), argv);
    }));
  };
}

}  // namespace

void tests_argparse() {
  tests_bool();
  tests_int();
  tests_double();
  tests_string<std::string>();
  tests_string<std::string_view>();
  tests_error();
}
