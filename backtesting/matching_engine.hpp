#pragma once

#include "Signals/Delegate.h"
#include "container.hpp"
#include "user_data.hpp"

namespace backtesting {
class order_book_t;
} // namespace backtesting

namespace matching_engine {
void matchOrder(backtesting::order_book_t &orderBook,
                backtesting::order_data_t const &order);
void cancelOrder(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order);

struct trade_signal_handler_t {
  using TradesDelegate = Gallant::Delegate1<backtesting::trade_list_t>;
  static utils::waitable_container_t<backtesting::trade_list_t> tradeList;

  static void OnNewTrades(backtesting::trade_list_t);
  static void OnNewTradesImpl();
  static TradesDelegate &GetTradesDelegate();
};
} // namespace matching_engine
