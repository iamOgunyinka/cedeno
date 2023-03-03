#pragma once

#include <cstdint>

#define CANDLESTICK "kline"
#define BTICKER "bookticker"
#define TICKER "ticker"
#define TRADE "trade"
#define DEPTH "depth"

#define SPOT "spot"
#define FUTURES "futures"

namespace backtesting {
enum class data_interval_e : uint32_t { // as defined by Binance
  one_second,                           // 1s
  one_minute,                           // 1m
  three_minutes,                        // 3m
  five_minutes,                         // 5m
  fifteen_minutes,                      // 15m
  thirty_minutes,                       // 30m
  one_hour,                             // 1h
  two_hours,                            // 2h
  four_hours,                           // 4h
  six_hours,                            // 6h
  twelve_hours,                         // 12h
  one_day,                              // 1d
  three_days,                           // 3d
  one_week,                             // 1w
  one_month                             // 1M
};

enum class trade_type_e : int {
  none,
  spot,
  futures,
};

enum class trade_side_e : int {
  none,
  sell,
  short_, // short is a reserved word
  buy,
  long_, // long is a reserved word
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

enum class time_duration_e : int {
  milliseconds,
  seconds,
};
// implemented in bookticker.cpp
bool requiresNewInsertion(uint64_t const &fresh, uint64_t const &old,
                          data_interval_e const interval,
                          time_duration_e const durationSpecifier);
} // namespace backtesting
