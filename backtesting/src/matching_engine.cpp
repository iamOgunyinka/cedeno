#include "matching_engine.hpp"
#include "spot_order_book.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>
#include <tuple>

std::map<int, std::vector<backtesting::recent_trades_callback_t>>
    recentTradesCallbacks;
std::map<int, std::vector<backtesting::aggregate_trades_callback_t>>
    aggTradesCallbacks;

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
utils::mutexed_list_t<backtesting::trade_list_t>
    trade_signal_handler_t::aggTradeList{};

void trade_signal_handler_t::onNewTrades(backtesting::trade_list_t trades) {
  aggTradeList.append(trades);
  tradeList.append(std::move(trades));
}

trade_signal_handler_t::TradesDelegate &
trade_signal_handler_t::GetTradesDelegate() {
  static std::unique_ptr<TradesDelegate> delegate_ = nullptr;
  if (!delegate_) {
    delegate_ = std::make_unique<TradesDelegate>();
    delegate_->Bind(trade_signal_handler_t::onNewTrades);

    std::thread{[] { onNewTradesImpl(); }}.detach();
    std::thread{[] { aggregateTradesImpl(); }}.detach();
  }
  return *delegate_;
}

void trade_signal_handler_t::onNewTradesImpl() {
  while (true) {
    auto trades = tradeList.get();
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

void trade_signal_handler_t::aggregateTradesImpl() {
  using backtesting::agg_trade_data_t;
  using backtesting::trade_data_t;
  using backtesting::trade_list_t;
  using backtesting::trade_side_e;

  struct comparator_t {
    bool operator()(std::string const &symbol, trade_data_t const &a) const {
      return symbol < a.tokenName;
    }
    bool operator()(trade_data_t const &a, std::string const &symbol) const {
      return a.tokenName < symbol;
    }
  };

  auto onNewAggregate = [](agg_trade_data_t const &agg) {
    if (!aggTradesCallbacks.empty()) {
      auto &callbacks = aggTradesCallbacks[(int)agg.tradeType];
      for (auto const &callback : callbacks) {
        if (callback)
          callback(agg);
      }
    }
  };

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (aggTradeList.isEmpty())
      continue;

    trade_list_t trades;
    while (!aggTradeList.isEmpty()) {
      auto temp = aggTradeList.get();
      trades.reserve(trades.size() + temp.size());
      trades.insert(trades.end(), temp.begin(), temp.end());
    }

    // sort by symbol name, followed by price, eventTime, then side
    std::sort(
        trades.begin(), trades.end(),
        [](trade_data_t const &a, trade_data_t const &b) {
          return std::tie(a.tokenName, a.amountPerPiece, a.eventTime, a.side) <
                 std::tie(b.tokenName, b.amountPerPiece, b.eventTime, b.side);
        });

    auto symbolStartIter = trades.begin();
    while (symbolStartIter != trades.end()) {
      auto symbolStopIter =
          std::upper_bound(symbolStartIter, trades.end(),
                           symbolStartIter->tokenName, comparator_t{});

      auto amountStartIter = symbolStartIter;
      while (amountStartIter != symbolStopIter) {
        auto amountStopIter = std::upper_bound(
            amountStartIter, symbolStopIter, amountStartIter->amountPerPiece,
            [](auto const &a, auto const &b) {
              using type_t = std::decay_t<std::remove_cv_t<decltype(a)>>;
              if constexpr (std::is_same_v<type_t, double>) {
                return a < b.amountPerPiece;
              } else {
                return a.amountPerPiece < b;
              }
            });

        agg_trade_data_t buyAggTrade;
        agg_trade_data_t sellAggTrade;

        for (auto iter = amountStartIter; iter != amountStopIter; ++iter) {
          auto &trade = iter->side == backtesting::trade_side_e::buy
                            ? buyAggTrade
                            : sellAggTrade;
          if (trade.eventTime == 0) {
            trade.eventTime = iter->eventTime;
            trade.firstTradeID = trade.aggTradeID = iter->tradeID;
            trade.price = iter->amountPerPiece;
            trade.tokenName = iter->tokenName;
            trade.tradeType = iter->tradeType;
          } else {
            trade.lastTradeID = iter->tradeID;
          }
          trade.quantity += iter->quantityExecuted;
        }
        if (amountStopIter == symbolStopIter)
          break;

        if (buyAggTrade.eventTime != 0)
          onNewAggregate(buyAggTrade);
        if (sellAggTrade.eventTime != 0)
          onNewAggregate(sellAggTrade);

        amountStartIter = amountStopIter;
      }
      if (symbolStopIter == trades.end())
        break;
      symbolStartIter = symbolStopIter;
    }
  }
}
} // namespace matching_engine

namespace backtesting {
void registerTradesCallback(backtesting::trade_type_e const tt,
                            backtesting::trades_event_callback_t cb,
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
