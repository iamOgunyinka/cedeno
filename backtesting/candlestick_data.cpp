#include "candlestick_data.hpp"
#include "common.hpp"

namespace backtesting {
candlestick_data_t candlestick_data_t::dataFromCSVLine(std::string const &str) {
  auto splittedString = backtesting::utils::splitString(str, ",");
  candlestick_data_t result;

  if (splittedString.size() != 16) {
    result.eventTime = 0;
    return result;
  }
  result.eventTime = std::stoull(splittedString[0]);
  result.startTime = std::stoull(splittedString[1]);
  result.closeTime = std::stoull(splittedString[2]);
  result.intervalInSeconds =
      backtesting::utils::stringToStdInterval(splittedString[3]);
  result.firstTradeID = std::stoull(splittedString[4]);
  result.lastTradeID = std::stoull(splittedString[5]);
  result.openPrice = std::stod(splittedString[6]);
  result.closePrice = std::stod(splittedString[7]);
  result.highPrice = std::stod(splittedString[8]);
  result.lowPrice = std::stod(splittedString[9]);
  result.baseAssetVolume = std::stod(splittedString[10]);
  result.numberOfTrades = std::stoul(splittedString[11]);
  result.klineIsClosed = std::stoul(splittedString[12]);
  result.quoteAssetVolume = std::stod(splittedString[13]);
  result.tbBaseAssetVolume = std::stod(splittedString[14]);
  result.tbQuoteAssetVolume = std::stod(splittedString[15]);
  return result;
}

void processCandlestickStream(trade_map_td const &tradeMap) {}
} // namespace backtesting
