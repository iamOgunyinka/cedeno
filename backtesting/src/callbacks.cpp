#include "callbacks.hpp"
#include "container.hpp"

#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>

namespace backtesting {

extern trades_callback_map_t recentTradesCallbacks;
extern agg_callback_map_t aggTradesCallbacks;

extern utils::waitable_container_t<trade_list_t> allNewTradeList;
extern utils::mutexed_list_t<trade_list_t> aggTradeList;

void trade_signal_handler_t::onNewTrades(trade_list_t trades) {
  aggTradeList.append(trades);
  allNewTradeList.append(std::move(trades));
}

void onNewTradesImpl();

trade_signal_handler_t::TradesDelegate &
trade_signal_handler_t::GetTradesDelegate() {
  static std::unique_ptr<TradesDelegate> delegate_ = nullptr;
  if (!delegate_) {
    delegate_ = std::make_unique<TradesDelegate>();
    delegate_->Bind(trade_signal_handler_t::onNewTrades);

    std::thread{[] { onNewTradesImpl(); }}.detach();
  }
  return *delegate_;
}

void onNewTradesImpl() {
  while (true) {
    auto trades = allNewTradeList.get();
    spdlog::info("Trades executed: {}", trades.size());

    if (!(recentTradesCallbacks.empty() && trades.empty())) {
      auto &callbacks = recentTradesCallbacks[(int)trades[0].tradeType];
      for (auto const &callback : callbacks) {
        if (callback)
          callback(trades);
      }
    }
  }
}

void registerTradesCallback(backtesting::trade_type_e const tt,
                            trades_event_callback_t cb,
                            bool const pushToFront) {
  // this will be removed when the futures orderBook is implemented
  if (tt != backtesting::trade_type_e::spot)
    return;

  std::visit(
      [t = (int)tt, pushToFront](auto &&cb) {
        using T = std::decay_t<decltype(cb)>;
        if constexpr (std::is_same_v<recent_trades_callback_t, T>) {
          auto &callbackList = recentTradesCallbacks[t];
          auto iter = std::find(callbackList.cbegin(), callbackList.cend(), cb);
          if (iter != callbackList.cend())
            return;
          if (!pushToFront)
            return callbackList.push_back(cb);
          callbackList.insert(callbackList.begin(), cb);
        } else if constexpr (std::is_same_v<aggregate_trades_callback_t, T>) {
          auto &callbackList = aggTradesCallbacks[t];
          auto iter = std::find(callbackList.cbegin(), callbackList.cend(), cb);
          if (iter != callbackList.cend())
            return;
          if (!pushToFront)
            return callbackList.push_back(cb);
          callbackList.insert(callbackList.begin(), cb);
        }
      },
      cb);
}
} // namespace backtesting
