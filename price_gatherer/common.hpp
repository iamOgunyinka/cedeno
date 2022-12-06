#pragma once
#include <optional>
#include <string>

namespace binance {

enum class trade_type_e {
  spot,
  futures,
};

struct url_t {
  static char const *const spotHostName;
  static char const *const futuresHostName;
  static char const *const spotPortNumber;
  static char const *const futuresPortNumber;
};

struct internal_token_data_t {
  std::string tokenName;
  bool subscribedFor = false;
};

struct candlestick_data_t {
  std::string tokenName;
  std::string interval;
  time_t startTime;
  time_t endTime;
  std::string openPrice;
  std::string closePrice;
  std::string highPrice;
  std::string lowPrice;
  std::string baseAssetVolume;
  std::string quoteAssetVolume;   // Quote asset volume
  std::string tbBaseAssetVolume;  // Taker buy base asset volume
  std::string tbQuoteAssetVolume; // Taker buy quote asset volume
  size_t numberOfTrades;
  bool klineIsClosed;
};

std::string toLowerString(std::string const &s);
std::optional<candlestick_data_t> parseCandleStickData(char const *str,
                                                       size_t const size);

} // namespace binance
