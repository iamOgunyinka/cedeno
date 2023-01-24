#include "matching_engine.hpp"
#include "order_book.hpp"
#include <memory>
#include <spdlog/spdlog.h>

std::map<int, std::vector<backtesting::new_trades_callback_t>>
    registeredCallbacks;

namespace matching_engine {

void matchOrder(backtesting::order_book_t &orderBook,
                backtesting::order_data_t const &order) {
  return orderBook.match(order);
}

void cancelOrder(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order) {
  return orderBook.cancel(order);
}

utils::waitable_container_t<backtesting::trade_list_t>
    trade_signal_handler_t::tradeList{};

void trade_signal_handler_t::OnNewTrades(backtesting::trade_list_t trades) {
  tradeList.append(std::move(trades));
}

trade_signal_handler_t::TradesDelegate &
trade_signal_handler_t::GetTradesDelegate() {
  static std::unique_ptr<TradesDelegate> delegate_ = nullptr;
  if (!delegate_) {
    delegate_ = std::make_unique<TradesDelegate>();
    delegate_->Bind(trade_signal_handler_t::OnNewTrades);

    std::thread{[] { OnNewTradesImpl(); }}.detach();
  }
  return *delegate_;
}

void trade_signal_handler_t::OnNewTradesImpl() {
  while (true) {
    auto trades = tradeList.get();
    spdlog::info("Trades executed: {}", trades.size());

    if (!(registeredCallbacks.empty() && trades.empty())) {
      auto &callbacks = registeredCallbacks[(int)trades[0].tradeType];
      for (auto const &callback : callbacks) {
        if (callback)
          callback(trades);
      }
    }
  }
}
} // namespace matching_engine

namespace backtesting {
void registerNewTradesCallback(backtesting::trade_type_e const tt,
                               backtesting::new_trades_callback_t cb,
                               bool const pushToFront) {
  // this will be removed when the futures orderBook is implemented
  if (tt != backtesting::trade_type_e::spot)
    return;

  auto &callbackList = registeredCallbacks[(int)tt];
  auto iter = std::find(callbackList.cbegin(), callbackList.cend(), cb);
  if (iter != callbackList.cend())
    return;

  if (!pushToFront)
    return callbackList.push_back(cb);
  callbackList.insert(callbackList.begin(), cb);
}
} // namespace backtesting
