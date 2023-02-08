#include "candlestick_data.hpp"
#include "global_data.hpp"
#include "trades_data.hpp"
#include <algorithm>
#include <filesystem>

namespace backtesting {
binance_candlestick_data_t
binance_candlestick_data_t::dataFromCSVLine(std::string const &str) {
  auto splittedString = backtesting::utils::splitString(str, ",");
  binance_candlestick_data_t result;

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

optional_kline_list_t getDiscreteKlineData(kline_config_t &&config) {
  if (config.interval < data_interval_e::one_second ||
      config.interval > data_interval_e::one_month)
    return std::nullopt;
  auto &allTradableTokens = global_data_t::instance().allTokens;
  config.symbol = utils::toUpperString(config.symbol);
  auto iter =
      std::lower_bound(allTradableTokens.begin(), allTradableTokens.end(),
                       config, [](auto const &a, auto const &b) {
                         using T = std::remove_cv_t<std::decay_t<decltype(a)>>;
                         if constexpr (std::is_same_v<T, kline_config_t>) {
                           return std::make_tuple(a.symbol, a.tradeType) <
                                  std::make_tuple(b.name, b.tradeType);
                         } else {
                           return std::make_tuple(a.name, a.tradeType) <
                                  std::make_tuple(b.symbol, b.tradeType);
                         }
                       });
  if (iter == allTradableTokens.end() ||
      !utils::isCaseInsensitiveStringCompare(iter->name, config.symbol))
    return std::nullopt;
  if (config.endTime == 0)
    config.endTime = std::time(nullptr);
  if (config.startTime == 0)
    config.startTime = config.endTime - 3'600;
  config.limit = std::clamp(config.limit, (int16_t)0, (int16_t)1'000);

  kline_data_list_t result;

  std::filesystem::path const rootPath(global_data_t::instance().rootPath);
  std::vector<std::time_t> intervals =
      utils::intervalsBetweenDates(config.startTime, config.endTime);
  if (intervals.empty())
    return result;

  if (intervals.back() < config.endTime)
    intervals.push_back(config.endTime);

  auto const tradeType = iter->tradeType == trade_type_e::spot ? SPOT : FUTURES;
  std::vector<std::filesystem::path> filePaths;
  filePaths.reserve(intervals.size());

  for (auto const &interval : intervals) {
    auto const date = utils::currentTimeToString(interval, "_").value();
    auto const path = rootPath / config.symbol / date / CANDLESTICK / tradeType;
    if (!std::filesystem::exists(path))
      continue;
    filePaths.push_back(path);
  }
  return result;
}

bool getContinuousKlineData(kline_config_t &&config) {
  //
  return false;
}

} // namespace backtesting
