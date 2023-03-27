#pragma once

#include <vector>
#include <string>
#include "enumerations.hpp"

namespace backtesting {
struct internal_token_data_t {
  std::string name;
  std::string baseAsset;
  std::string quoteAsset;
  trade_type_e tradeType = trade_type_e::none;
};
using token_data_list_t = std::vector<internal_token_data_t>;

}