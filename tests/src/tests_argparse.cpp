#include "tests_argparse.h"

#include <sh/argparse.h>

void tests_argparse() {
  sh::argument_parser parser;
  parser.add<int>("-v", "--version") | sh::help("which version?") | 10;
}
