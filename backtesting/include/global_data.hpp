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
  static int64_t newUser(backtesting::spot_wallet_asset_list_t tokensOwned);
  static global_data_t &instance();
};
