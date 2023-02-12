#include "../data_extractor/data_extractor.hpp"
#include <cassert>
#include <iostream>
#include <vector>

#include <thread>

int main(int argc, char **argv) {
  data_extractor_t extractor;
  bool const result = extractor.run(argc, argv);

  assert(result == 0);
  std::cout << "Done running" << std::endl;
  return 0;
}
