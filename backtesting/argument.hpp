#pragma once

#include <string>
#include <vector>

namespace backtesting {
std::string getDatabaseConfigPath();
}

struct argument_t {
  using stringlist_t = std::vector<std::string>;

  stringlist_t streams;
  stringlist_t tradeTypes;
  stringlist_t tokenList;

  std::string rootDir;
  std::string dateFromStr;
  std::string dateToStr;
  std::string dbConfigFilename = backtesting::getDatabaseConfigPath();
  std::string dbLaunchType = "development";
};
