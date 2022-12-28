#include "../data_extractor/data_extractor.hpp"
#include <cassert>
#include <iostream>
#include <thread>

int main(int argc, char const **argv) {
  std::vector<std::string> args;
  args.reserve(argc);

  for (int i = 0; i < argc; ++i)
    args.push_back(argv[i]);

  data_extractor_t extractor;

  // run the extractor for only 60 seconds
  std::thread([&extractor] {
    std::this_thread::sleep_for(std::chrono::seconds(60));
    assert(extractor.stop());
    std::cout << "Stopped" << std::endl;
  }).detach();

  assert(extractor.run(std::move(args)) == 0);
  std::cout << "Done running" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return 0;
}
