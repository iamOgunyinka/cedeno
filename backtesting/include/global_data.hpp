#pragma once

#include "common.hpp"
#include "user_data.hpp"
#include <set>

struct global_data_t {
  global_data_t() {}

  unsigned long long startTime = 0;
  unsigned long long endTime = 0;
  backtesting::filename_map_td listOfFiles;
  backtesting::token_data_list_t allTokens;
  backtesting::user_data_list_t allUserAccounts;
  std::set<std::string> validSymbols;
#ifdef BT_USE_WITH_INDICATORS
  /// a list of configurations used by the indicators
  std::vector<std::vector<std::string>> indicatorConfig;
#endif

  std::string rootPath;

  double futuresMakerFee = 0.0;
  double futuresTakerFee = 0.0;
  double spotMakerFee = 0.0;
  double spotTakerFee = 0.0;

  static int64_t newUser(backtesting::wallet_asset_list_t tokensOwned);
  static global_data_t &instance();
};
