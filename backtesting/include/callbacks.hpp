#pragma once

#include "Signals/Delegate.h"
#include "aggregate_trade.hpp"
#include "depth_data.hpp"
#include "trades_data.hpp"

#include <variant>

/// @typedef
/**< a variant of callbacks for recent trades and aggregate trades */
using trades_event_callback_t =
    std::variant<backtesting::recent_trades_callback_t,
                 backtesting::aggregate_trades_callback_t>;

namespace backtesting {
/// call this function to register callbacks that receives updates on spot &
/// futures trades \param tt specifies the trade type (futures or spot) \param
/// cb the callback to register \param pushToFront a boolean to specify where in
/// the callback queue the callback will be placed
void registerTradesCallback(backtesting::trade_type_e const tt,
                            trades_event_callback_t cb,
                            bool const pushToFront = true);
/// call this function to register callbacks for depth data
/// \param tt specifies the trade type (futures or spot)
/// \param cb the callback to register
/// \param pushToFront a boolean to specify where in the callback queue the
/// callback will be placed
void registerDepthCallback(backtesting::trade_type_e const tt,
                           depth_event_callback_t cb,
                           bool const pushToFront = true);

/// internal class in charge of trades' signals and delegates
struct trade_signal_handler_t {
  using TradesDelegate = Gallant::Delegate1<trade_list_t>;

  /// called whenever a trade occurs on any open order book
  static void onNewTrades(trade_list_t);
  /// internal class in charge of connecting the different order books to
  /// handlers \return a delegate that is bound to an handler
  static TradesDelegate &GetTradesDelegate();
};

/// internal class in charge of reading depth from file and sending to user
struct depth_signal_handler_t {
  /// \typedef internal typedef for depth delegate
  using DepthDelegate = Gallant::Delegate1<depth_data_t>;

  /// called whenever a new depth is read and ready to be signalled to the user
  static void onNewDepthObtained(depth_data_t);
  /// internal class in charge of connecting depth signals to accompanying
  /// delegates \return a delegate that is bound to an handler
  static DepthDelegate &GetDepthDelegate();
};

/// an internal function that is used by a detached thread handling reading of
/// depths
void onNewDepthThreadImpl();
} // namespace backtesting
