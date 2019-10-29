#include <all_type_variant.hpp>
#include <type_cast.hpp>
#include <iostream>

#include "../lib/utils/assert.hpp"


opossum::AllTypeVariant foo = 4; // now storing an int
opossum::AllTypeVariant giveFloat() { return 4.3f; }
opossum::AllTypeVariant giveInt() { return 5; }

int main() {
  opossum::Assert(true, "We can use opossum files here :)");


  foo = giveFloat();
  std::cout << foo << ", " << giveInt() << std::endl;
  float bar = opossum::type_cast<float>(giveInt());
  std::cout << bar;

  return 0;
}
