#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace backtesting {
enum class trade_type_e : int {
  none,
  spot,
  futures,
};

enum class trade_side_e : int {
  none,
  sell, // short
  buy,  // long
};

enum class trade_market_e : int {
  none,
  limit,
  market,
};
using market_type_e = trade_market_e;

enum class order_status_e : int {
  new_order,
  partially_filled,
  filled,
  cancelled,
  pending_cancel,
  rejected,
  expired,
};

struct trade_data_t {
  std::string tokenName;
  uint64_t tradeID = 0;
  uint64_t orderID = 0;
  uint64_t eventTime = 0;
  double quantityExecuted = 0.0;
  double amountPerPiece = 0.0;
  trade_side_e side = trade_side_e::none;
  trade_type_e tradeType = trade_type_e::none;
  order_status_e status = order_status_e::new_order;
};
using trade_list_t = std::vector<trade_data_t>;

using recent_trades_callback_t = void (*)(trade_list_t const &);
using trades_callback_map_t =
    std::map<int, std::vector<recent_trades_callback_t>>;
} // namespace backtesting
