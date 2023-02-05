#pragma once

#include "common.hpp"

namespace backtesting {

struct candlestick_data_t {
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

  static candlestick_data_t dataFromCSVLine(std::string const &str);
};

void processCandlestickStream(trade_map_td const &tradeMap);

} // namespace backtesting
