#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "common.hpp"
#include "data_streamer.hpp"

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost

namespace net = boost::asio;

namespace backtesting {
struct depth_data_t {
  struct depth_meta_t {
    double priceLevel = 0.0;
    double quantity = 0.0;
  };

  std::string tokenName;
  time_t eventTime = 0;
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

void processDepthStream(net::io_context &ioContext, trade_map_td &tradeMap);

} // namespace backtesting
