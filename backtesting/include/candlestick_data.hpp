#pragma once

#include "common.hpp"

namespace backtesting {

struct binance_candlestick_data_t {
  std::chrono::seconds intervalInSeconds;
  uint64_t eventTime;
  uint64_t startTime;
  uint64_t closeTime;
  uint64_t firstTradeID;
  uint64_t lastTradeID;
  double openPrice;
  double closePrice;
  double highPrice;
  double lowPrice;
  double baseAssetVolume;
  double quoteAssetVolume;
  double tbBaseAssetVolume;  // Taker buy base asset volume
  double tbQuoteAssetVolume; // Taker buy quote asset volume
  size_t numberOfTrades;
  size_t klineIsClosed;

  static binance_candlestick_data_t dataFromCSVLine(std::string const &str);
};

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

// forward declaration
enum class trade_type_e;

struct continuous_kline_data_t {
  std::string symbol;
};

struct kline_data_t {
  double ts = 0.0; // Event time
  double openPrice = 0.0;
  double highPrice = 0.0;
  double lowPrice = 0.0;
  double closePrice = 0.0;
  double baseVolume = 0.0;
  double quoteVolume = 0.0;
};
using kline_data_list_t = std::vector<kline_data_t>;
using optional_kline_list_t = std::optional<kline_data_list_t>;
using kline_callback_t = void (*)(continuous_kline_data_t const &);

struct kline_config_t {
  std::string symbol;
  data_interval_e interval = data_interval_e::one_hour;
  trade_type_e tradeType;
  time_t startTime = 0;
  time_t endTime = 0;
  int16_t limit = 500; // max 1000
  // only available for continuous data, not discrete
  kline_callback_t callback = nullptr;
};

optional_kline_list_t getDiscreteKlineData(kline_config_t &&config);
bool getContinuousKlineData(kline_config_t &&config);
} // namespace backtesting
