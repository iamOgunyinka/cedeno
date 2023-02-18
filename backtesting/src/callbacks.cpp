#include "callbacks.hpp"
#include "container.hpp"

#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>

namespace backtesting {

extern trades_callback_map_t recentTradesCallbacks;
extern agg_callback_map_t aggTradesCallbacks;
extern depth_callback_map_t depthCallbackList;
extern ::utils::mutexed_list_t<trade_list_t> aggTradeList;
extern ::utils::waitable_container_t<trade_list_t> allNewTradeList;
extern ::utils::waitable_container_t<depth_data_t> depthDataList;

void trade_signal_handler_t::onNewTrades(trade_list_t trades) {
  aggTradeList.append(trades);
  allNewTradeList.append(std::move(trades));
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

depth_signal_handler_t::DepthDelegate &
depth_signal_handler_t::GetDepthDelegate() {
  static std::unique_ptr<DepthDelegate> delegate_ = nullptr;
  if (!delegate_) {
    delegate_ = std::make_unique<DepthDelegate>();
    delegate_->Bind(depth_signal_handler_t::onNewDepthObtained);

    std::thread{[] { onNewDepthThreadImpl(); }}.detach();
  }
  return *delegate_;
}

void registerDepthCallback(backtesting::trade_type_e const tt,
                           depth_event_callback_t cb, bool const pushToFront) {
  // this will be removed when the futures orderBook is implemented
  if (tt != backtesting::trade_type_e::spot)
    return;

  auto &callbackList = depthCallbackList[(int)tt];
  auto iter = std::find(callbackList.cbegin(), callbackList.cend(), cb);
  if (iter != callbackList.cend())
    return;

  if (!pushToFront)
    return callbackList.push_back(cb);
  callbackList.insert(callbackList.begin(), cb);
}

void registerTradesCallback(backtesting::trade_type_e const tt,
                            trades_event_callback_t cb,
                            bool const pushToFront) {
  // this will also be removed when the futures orderBook is implemented
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

void depth_signal_handler_t::onNewDepthObtained(depth_data_t depth) {
  depthDataList.append(std::move(depth));
}

void onNewDepthThreadImpl() {
  while (true) {
    auto data = depthDataList.get();

    spdlog::info("Asks: {}, Bids: {}", data.asks.size(), data.bids.size());

    if (!(recentTradesCallbacks.empty() && data.eventTime > 0)) {
      auto &callbacks = depthCallbackList[(int)data.tradeType];
      for (auto const &callback : callbacks) {
        if (callback)
          callback(depthDataToPythonDepth(data));
      }
    }
  }
}

} // namespace backtesting
