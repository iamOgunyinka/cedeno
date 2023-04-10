#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "data_streamer.hpp"
#include "enumerations.hpp"

namespace backtesting {

struct exchange_trade_data_t {
  uint64_t eventTimeMs{};
  uint64_t tradeTimeMs{};
  uint64_t aggregateTradeID = 0;
  uint64_t firstTradeID = 0;
  uint64_t lastTradeID = 0;
  double price = 0;
  double quantity = 0;
  bool isBuyerMarketMaker = false;

  static exchange_trade_data_t
  dataFromCSVStream(data_streamer_t<exchange_trade_data_t> &dataStreamer);

private:
  template <typename T> static T getNumber(csv::CSVRow::iterator const &iter) {
    return iter->is_str() ? std::stoull(iter->get_sv().data()) : iter->get<T>();
  }
  static exchange_trade_data_t tradeFromCSVRow(csv::CSVRow const &row);
  static bool isExpectedRowCount(size_t const r) { return r == 8; }
};
using reader_trade_data_t = exchange_trade_data_t;

struct trade_data_t {
  std::string tokenName;
  uint64_t tradeID = 0;
  uint64_t orderID = 0;
  uint64_t eventTime = 0;
  double quantityExecuted = 0.0;
  double amountPerPiece = 0.0;
  trade_side_e side = trade_side_e::none;
  trade_type_e tradeType = trade_type_e::none;
  order_status_e status = order_status_e::new_order;
};
using trade_list_t = std::vector<trade_data_t>;

exchange_trade_data_t localToExchangeTrade(trade_data_t const &,
                                           bool isMarketMaker);

using recent_trades_callback_t = std::function<void(trade_list_t const &)>;
using trades_callback_map_t =
    std::map<int, std::vector<recent_trades_callback_t>>;
} // namespace backtesting
