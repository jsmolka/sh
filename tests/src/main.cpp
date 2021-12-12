#include "tests_argparse.h"
#include "tests_array.h"
#include "tests_filesystem.h"
#include "tests_hash.h"
#include "tests_parse.h"
#include "tests_ranges.h"
#include "tests_stack.h"
#include "tests_utility.h"
#include "tests_vector.h"

int main() {
  tests_argparse();
  tests_array();
  tests_filesystem();
  tests_hash();
  tests_parse();
  tests_ranges();
  tests_stack();
  tests_utility();
  tests_vector();
}
