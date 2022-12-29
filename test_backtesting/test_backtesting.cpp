#include "../backtesting/arguments_parser.hpp"

int main(int argc, char **argv) {

  std::vector<std::string> args;
  for (int i = 0; i < argc; ++i)
    args.push_back(argv[i]);

  argument_parser_t parser{};
  if (!(parser.parse(std::move(args)) && parser.prepareData()))
    return EXIT_FAILURE;
  return parser.runBacktester();
}
