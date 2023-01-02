#include "../backtesting/arguments_parser.hpp"

int main(int argc, char **argv) {

  argument_parser_t parser{};
  if (!(parser.parse(argc, argv) && parser.prepareData()))
    return EXIT_FAILURE;
  return parser.runBacktester();
}
