#pragma once

#include "common.hpp"
#include "container.hpp"
#include "data_streamer.hpp"
#include <functional>

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

  static binance_candlestick_data_t
  dataFromCSVStream(data_streamer_t<binance_candlestick_data_t> &dataStreamer);

private:
  template <typename T> static T getNumber(csv::CSVRow::iterator const &iter) {
    return iter->is_str() ? std::stoull(iter->get_sv().data()) : iter->get<T>();
  }

  template <typename T>
  static T getTimeInSecs(csv::CSVRow::iterator const &iter) {
    return getNumber<T>(iter) / static_cast<T>(1'000);
  }

  static bool isExpectedRowCount(size_t const r) { return r == 16; }
  static binance_candlestick_data_t klineFromCSVRow(csv::CSVRow const &row);
};

struct kline_data_t {
  uint64_t ts = 0; // Event time
  uint64_t ntrades = 1;
  double openPrice = 0.0;
  double highPrice = 0.0;
  double lowPrice = 0.0;
  double closePrice = 0.0;
  double baseVolume = 0.0;
  double quoteVolume = 0.0;

  kline_data_t &operator+=(kline_data_t const &);
};
using kline_data_list_t = std::vector<kline_data_t>;
using optional_kline_list_t = std::optional<kline_data_list_t>;
using kline_callback_t = std::function<void(kline_data_list_t const &)>;

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

struct kline_task_t {
  data_interval_e interval = data_interval_e::one_hour;
  kline_callback_t callback = nullptr;
  data_streamer_t<binance_candlestick_data_t> dataStream;

  kline_task_t(data_streamer_t<binance_candlestick_data_t> &&d)
      : dataStream(std::move(d)) {}

  static ::utils::waitable_container_t<kline_task_t> klineScheduledTasks;
};

optional_kline_list_t getDiscreteKlineData(kline_config_t &&config);
bool checkAndValidateKlineRequest(kline_config_t &config);
bool getContinuousKlineData(kline_config_t &&config);
void candlestickProcessingImpl();
void klineChildThreadImpl(kline_task_t &&);
} // namespace backtesting
