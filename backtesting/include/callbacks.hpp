#pragma once

#include "Signals/Delegate.h"
#include "aggregate_trade.hpp"
#include "trades_data.hpp"

#include <variant>

using trades_event_callback_t =
    std::variant<backtesting::recent_trades_callback_t,
                 backtesting::aggregate_trades_callback_t>;

namespace backtesting {
void registerTradesCallback(backtesting::trade_type_e const tt,
                            trades_event_callback_t cb,
                            bool const pushToFront = true);

struct trade_signal_handler_t {
  using TradesDelegate = Gallant::Delegate1<trade_list_t>;

  static void onNewTrades(trade_list_t);
  static TradesDelegate &GetTradesDelegate();
};
} // namespace backtesting
