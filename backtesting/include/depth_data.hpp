#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "common.hpp"
#include "data_streamer.hpp"

namespace backtesting {
struct depth_data_t {
  struct depth_meta_t {
    double priceLevel = 0.0;
    double quantity = 0.0;
  };

  std::string tokenName;
  time_t eventTime = 0;
  trade_type_e tradeType = trade_type_e::none;
  uint64_t firstUpdateID = 0;
  uint64_t finalUpdateID = 0;
  uint64_t finalStreamUpdateID = 0;
  std::vector<depth_meta_t> bids;
  std::vector<depth_meta_t> asks;

  static depth_data_t
  dataFromCSVStream(data_streamer_t<depth_data_t> &dataStreamer);

private:
  template <typename T> static T getNumber(csv::CSVRow::iterator const &iter) {
    return iter->is_str() ? std::stoull(iter->get_sv().data()) : iter->get<T>();
  }
  static bool depthMetaFromCSV(csv::CSVRow const &row, depth_data_t &data);
  static depth_data_t depthFromCSVRow(csv::CSVRow const &row);
};

struct py_depth_data_t {
  uint64_t eventTime = 0;
  int type = 0;
  double price = 0.0;
  double quantity = 0.0;
};
using py_depth_data_list_t = std::vector<py_depth_data_t>;

void processDepthStream(trade_map_td &tradeMap);
py_depth_data_list_t depthDataToPythonDepth(depth_data_t const &);
using depth_event_callback_t = void (*)(py_depth_data_list_t);
using depth_callback_map_t = std::map<int, std::vector<depth_event_callback_t>>;
} // namespace backtesting
