#pragma once

#include "common.hpp"
#include <pybind11/stl.h>

namespace backtesting {
std::string getDatabaseConfigPath();

struct argument_t {
  using stringlist_t = std::vector<std::string>;

  stringlist_t streams;
  stringlist_t tradeTypes;
  stringlist_t tokenList;

  std::string rootDir;
  std::string dateFromStr;
  std::string dateToStr;
  std::string dbConfigFilename = getDatabaseConfigPath();
  std::string dbLaunchType = "development";
};

} // namespace backtesting

class argument_parser_t {
public:
  bool parse(size_t argc, char **argv);
  bool prepareData();
  bool isReady() const { return m_argumentParsed && m_authenticatedData; }
  int runBacktester();

private:
  std::optional<backtesting::argument_t> m_args;
  bool m_argumentParsed = false;
  bool m_authenticatedData = false;
};
