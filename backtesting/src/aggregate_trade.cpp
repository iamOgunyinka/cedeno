#include "aggregate_trade.hpp"

#include "container.hpp"
#include "trades_data.hpp"
#include <algorithm>
#include <map>
#include <thread>
#include <vector>

namespace backtesting {

agg_callback_map_t aggTradesCallbacks{};
utils::mutexed_list_t<backtesting::trade_list_t> aggTradeList;

void aggregateTradesImpl() {
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
} // namespace backtesting
