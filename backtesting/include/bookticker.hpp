#pragma once

#include "common.hpp"
#include "container.hpp"
#include "data_streamer.hpp"
#include <array>
#include <functional>

namespace backtesting {

struct binance_bktick_data_t {
  double orderBookUpdateID;
  double bestBidPrice;
  double bestBidQty;
  double bestAskPrice;
  double bestAskQty;
  uint64_t eventTimeInMs;
  uint64_t transactionTimeInMs;

  static binance_bktick_data_t
  dataFromCSVStream(data_streamer_t<binance_bktick_data_t> &dataStreamer);

private:
  template <typename T> static T getNumber(csv::CSVRow::iterator const &iter) {
    return iter->is_str() ? std::stoull(iter->get_sv().data()) : iter->get<T>();
  }

  static bool isExpectedRowCount(size_t const r) { return r == 7; }
  static binance_bktick_data_t bookTickerFromCSVRow(csv::CSVRow const &row);
};

struct bktick_data_t {
  uint64_t ts; // Event time
  double bestBidPrice;
  double bestBidQty;
  double bestAskPrice;
  double bestAskQty;
  std::string symbol;
};

using bktick_list_t = std::vector<bktick_data_t>;
using bktick_callback_t = std::function<void(bktick_data_t const &)>;

struct bktick_config_t {
  std::vector<std::string> symbols;
  trade_type_e tradeType;
  time_t startTime = 0;
  time_t endTime = 0;
  bktick_callback_t callback = nullptr;

  static ::utils::waitable_container_t<bktick_config_t> bookTickerConfigs;
  // globalBkTickerRecord[0] == spot
  // globalBkTickerRecord[1] == futures
  static std::array<std::vector<bktick_data_t>, 2> globalBkTickerRecord;
  static std::mutex globalBkTickerMutex;
};

bool checkAndValidateBookTickerRequestHelper(bktick_config_t &config);
bool checkAndValidateBookTickerRequest(bktick_config_t &config);
bool getContinuousBTickerData(bktick_config_t &&config);
bktick_list_t getDiscreteBTickerData(bktick_config_t &&config);
void bookTickerProcessingThreadImpl(bktick_config_t &&config);
void insertNewestData(bktick_data_t const &, bktick_config_t const &);
void bookTickerChildThreadImpl(bktick_config_t &&);
} // namespace backtesting
