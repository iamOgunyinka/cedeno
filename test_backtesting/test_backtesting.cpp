#include "arguments_parser.hpp"

int main(int argc, char **argv) {

  /*** TEST 1
  {
    backtesting_t parser{};
    if (!(parser.parse(argc, argv) && parser.prepareData()))
      return EXIT_FAILURE;
    (void)parser.run();
  }
  ***/

  {
    std::string const &filename =
        "D:\\Visual Studio Projects\\"
        "cedeno\\test_backtesting\\config\\config.ini";
    if (!backtesting::createBTInstanceFromConfigFile(filename))
      return EXIT_FAILURE;
    if (auto *bt = backtesting::getGlobalBTInstance(); bt != nullptr)
      bt->run();
  }

  return EXIT_SUCCESS;
}
