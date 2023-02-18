#pragma once

#include "Signals/Delegate.h"
#include "aggregate_trade.hpp"
#include "depth_data.hpp"
#include "trades_data.hpp"

#include <variant>

using trades_event_callback_t =
    std::variant<backtesting::recent_trades_callback_t,
                 backtesting::aggregate_trades_callback_t>;

namespace backtesting {
void registerTradesCallback(backtesting::trade_type_e const tt,
                            trades_event_callback_t cb,
                            bool const pushToFront = true);
void registerDepthCallback(backtesting::trade_type_e const tt,
                           depth_event_callback_t cb,
                           bool const pushToFront = true);

struct trade_signal_handler_t {
  using TradesDelegate = Gallant::Delegate1<trade_list_t>;

  static void onNewTrades(trade_list_t);
  static TradesDelegate &GetTradesDelegate();
};

struct depth_signal_handler_t {
  using DepthDelegate = Gallant::Delegate1<depth_data_t>;

  static void onNewDepthObtained(depth_data_t);
  static DepthDelegate &GetDepthDelegate();
};
void onNewDepthThreadImpl();
} // namespace backtesting
