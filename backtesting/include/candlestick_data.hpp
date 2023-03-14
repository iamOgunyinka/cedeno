#pragma once

#include "common.hpp"
#include "container.hpp"
#include "data_streamer.hpp"
#include <functional>

namespace backtesting {
/// This is an internal data structure. It is the data structure of the candlestick
/// data read from the file and processed
struct binance_candlestick_data_t {
  std::chrono::seconds intervalInSeconds; /*!< intervals in seconds between events */
  uint64_t eventTime; /*!< the time event occurs */
  uint64_t startTime; /*!< trade start time */
  uint64_t closeTime; /*!< trade close time */
  uint64_t firstTradeID; /*!< first trade ID */
  uint64_t lastTradeID; /*!< last trade ID */
  double openPrice; /*!< trade open price */
  double closePrice; /*!< trade closing price */
  double highPrice; /*!< highest price of instrument */
  double lowPrice; /*!< lowest price of instrument */
  double baseAssetVolume; /*!< base asset volume */
  double quoteAssetVolume; /*!< quote asset volume */
  double tbBaseAssetVolume;   /*!< Taker buy base asset volume */
  double tbQuoteAssetVolume;  /*!< Taker buy quote asset volume */
  size_t numberOfTrades;  /*!< total number of trades */
  size_t klineIsClosed;  /*!< is candlestick/kline closed? */

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

/// This is the kline data structure returned to the user.
struct kline_data_t {
  uint64_t ts = 0; /// Event time
  uint64_t ntrades = 1; /// number of trades making up this data
  double openPrice = 0.0; /// open price
  double highPrice = 0.0; /// high price
  double lowPrice = 0.0; /// low price
  double closePrice = 0.0; /// close price
  double baseVolume = 0.0; /// base volume
  double quoteVolume = 0.0; /// quote volume

  kline_data_t &operator+=(kline_data_t const &);
};
using kline_data_list_t = std::vector<kline_data_t>;
using optional_kline_list_t = std::optional<kline_data_list_t>;
using kline_callback_t = std::function<void(kline_data_list_t const &)>;

/// This is the kline configuration sent from the user (most likely from the Python script)
/// specifying the symbol needed, start and end time, trade type and an optional
/// callback to get periodic data
struct kline_config_t {
  std::string symbol;
  data_interval_e interval = data_interval_e::one_hour;
  trade_type_e tradeType;
  time_t startTime = 0;
  time_t endTime = 0;
  int16_t limit = 500; /// max 1000
  /// only available for continuous data, not discrete
  kline_callback_t callback = nullptr;
};

/// internal data structure encompassing a kline task
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
