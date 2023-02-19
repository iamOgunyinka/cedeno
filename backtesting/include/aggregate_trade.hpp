#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace backtesting {
enum class trade_type_e;

struct agg_trade_data_t {
  std::string tokenName;
  uint64_t aggTradeID = 0;
  uint64_t orderID = 0;
  uint64_t eventTime = 0;
  uint64_t firstTradeID = 0;
  uint64_t lastTradeID = 0;
  double price = 0.0;
  double quantity = 0.0;
  trade_type_e tradeType;
};

void aggregateTradesImpl();

using aggregate_trades_callback_t =
    std::function<void(agg_trade_data_t const &)>;
using agg_callback_map_t =
    std::map<int, std::vector<aggregate_trades_callback_t>>;
} // namespace backtesting
