#pragma once

#include <ctime>
#include <functional>
#include <string>
#include <vector>

#include "common.hpp"
#include "data_streamer.hpp"

namespace backtesting {
class order_book_base_t;

struct global_order_book_t {
  std::string tokenName;
  std::unique_ptr<order_book_base_t> futures;
  std::unique_ptr<order_book_base_t> spot;
  static std::vector<global_order_book_t> globalOrderBooks;
};

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

py_depth_data_list_t depthDataToPythonDepth(depth_data_t const &);
void processDepthStream(trade_map_td &tradeMap
#ifdef BT_USE_WITH_INDICATORS
    , std::vector<std::vector<std::string>> &&config
#endif
);

using depth_event_callback_t = std::function<void(py_depth_data_list_t)>;
using depth_callback_map_t = std::map<int, std::vector<depth_event_callback_t>>;
} // namespace backtesting
