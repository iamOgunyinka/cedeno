#include "../data_extractor/data_extractor.hpp"
#include <cassert>
#include <iostream>
#include <vector>

#include <thread>

int main() {
  data_extractor_t extractor;
  int argc = 3;

  std::vector<const char *> a{"./test_data_extractor",
                              "--trade-type",
                              "futures",
                              "spot",
                              "--streams",
                              "bookticker",
                              "-r",
                              "120"};

  bool const result = extractor.run(a.size(), const_cast<char **>(&a[0]));
  assert(result == 0);
  std::cout << "Done running" << std::endl;
  return 0;
}
