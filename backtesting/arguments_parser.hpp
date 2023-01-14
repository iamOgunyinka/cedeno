#pragma once

#include "common.hpp"

namespace backtesting {
enum class trade_type_e;

#ifdef BT_USE_WITH_DB
std::string getDatabaseConfigPath();
#endif // BT_USE_WITH_DB

struct configuration_t {
  using stringlist_t = std::vector<std::string>;

  stringlist_t streams;
  stringlist_t tradeTypes;
  stringlist_t tokenList;

  std::string dateFromStr;
  std::string dateToStr;
  std::string rootDir;

#ifdef BT_USE_WITH_DB
  std::string dbConfigFilename = getDatabaseConfigPath();
  std::string dbLaunchType = "development";
#endif
};

/*
void readTokensFromFileImpl(token_data_list_t& result,
                            trade_type_e const tradeType,
                            std::string const &filename);
*/
} // namespace backtesting

class backtesting_t {
  bool isReady() const { return m_argumentParsed && m_authenticatedData; }
  bool parseImpl(backtesting::configuration_t);

public:
  backtesting_t();
  backtesting_t(backtesting::configuration_t const &);
  bool parse(size_t argc, char **argv);
  bool prepareData();
  int run();

  friend std::optional<backtesting_t>
  newBTInstance(backtesting::configuration_t const &);

private:
  std::optional<backtesting::configuration_t> m_config;
  bool m_argumentParsed = false;
  bool m_authenticatedData = false;
};

std::optional<backtesting_t>
newBTInstance(backtesting::configuration_t const &);
