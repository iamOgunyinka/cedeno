#include "matching_engine.hpp"
#include "order_book.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace matching_engine {

void match_order(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order) {
  return orderBook.match(order);
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
    spdlog::info("Trades executed: ", trades.size());
  }
}
} // namespace matching_engine
