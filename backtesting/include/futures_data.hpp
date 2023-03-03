#pragma once

#include "enumerations.hpp"

namespace backtesting {
namespace futures {

struct futures_order_t {
  uint64_t traderID = 0;
  trade_side_e side = trade_side_e::none;
  trade_market_e market = trade_market_e::none;
  double price = 0.0;
  double quantity = 0.0;
  double leverage = 0.0;
  bool is_cancelled = false;
};

struct futures_trade_t {
  uint64_t tradeID = 0;
  trade_side_e side = trade_side_e::none;
  double price = 0.0;
  double quantity = 0.0;
  double leverage = 0.0;
};

struct futures_trader_t {};
} // namespace futures
} // namespace backtesting
