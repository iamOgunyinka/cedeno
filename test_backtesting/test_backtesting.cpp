#include "arguments_parser.hpp"

int main(int argc, char **argv) {

  backtesting_t parser{};
  if (!(parser.parse(argc, argv) && parser.prepareData()))
    return EXIT_FAILURE;
  return parser.run();
}
