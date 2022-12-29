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
  long long const t = args.size() >= 2 ? std::stoll(args[1]) : 120;
  // run the extractor for only 60 seconds
  std::thread([&extractor, t] {
    std::this_thread::sleep_for(std::chrono::seconds(t));
    assert(extractor.stop());
    std::cout << "Stopped" << std::endl;
  }).detach();

  assert(extractor.run(std::move(args)) == 0);
  std::cout << "Done running" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  return 0;
}
