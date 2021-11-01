#define BOOST_UT_DISABLE_MODULE
#include <boost/ut.hpp>

auto sum(auto... args) { return (args + ...); }

int main() {
  using namespace boost::ut;

  "sum"_test = [] {
    expect(sum(1, 2) == 3_i);
    expect(sum(1, 3) == 4_i);
  };
}
