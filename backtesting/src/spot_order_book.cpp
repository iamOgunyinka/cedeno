#include "spot_order_book.hpp"
#include <mutex>

#ifdef _DEBUG
#include <spdlog/spdlog.h>
#endif

namespace backtesting {
static details::lesser_comparator_t lesserComparator{};
static details::greater_comparator_t greaterComparator{};

spot_order_book_t::spot_order_book_t(net::io_context &ioContext,
                                     data_streamer_t<depth_data_t> dataStreamer,
                                     internal_token_data_t *symbol)
    : order_book_base_t(ioContext, std::move(dataStreamer), symbol,
                        trade_type_e::spot) {}

void spot_order_book_t::cancel(order_data_t order) {
  static auto findAndCancelOrder = [this](auto &sides, order_data_t &&order,
                                          auto comparator) {
    auto iter = std::lower_bound(sides.begin(), sides.end(), order.priceLevel,
                                 comparator);
    if (iter == sides.end() || iter->priceLevel != order.priceLevel)
      return;
    auto &allOrders = iter->orders;
    auto findIter =
        std::find_if(allOrders.begin(), allOrders.end(),
                     [orderID = order.orderID](order_data_t const &a) {
                       return a.orderID == orderID;
                     });
    if (findIter == allOrders.end())
      return;
    iter->totalQuantity -= findIter->quantity;
    (void)getNewTrade(order, order_status_e::cancelled, findIter->quantity,
                      order.priceLevel);
    allOrders.erase(findIter);
  };

  if (order.side == trade_side_e::buy) {
    return findAndCancelOrder(m_orderBook.bids, std::move(order),
                              greaterComparator);
  }

  return findAndCancelOrder(m_orderBook.asks, std::move(order),
                            lesserComparator);
}

void spot_order_book_t::match(order_data_t order) {
  auto const isBuying = (order.side == trade_side_e::buy);
  auto &bids = m_orderBook.bids;
  auto &asks = m_orderBook.asks;

  auto broadcastTradeSignal = [this](trade_list_t &&result) {
    NewTradesCreated(std::move(result));
  };

  trade_list_t result;
  if (order.market == trade_market_e::market) {
    result = marketMatcher(isBuying ? asks : bids, order.priceLevel, order);
  } else if (order.market == trade_market_e::limit) {
    if (isBuying) {
      double &qty = order.quantity;
      if (asks.empty() ||
          (!bids.empty() && order.priceLevel <= bids.front().priceLevel) ||
          (!asks.empty() && asks.front().priceLevel > order.priceLevel)) {
        updateSidesWithNewOrder(order, bids, greaterComparator);
        return broadcastTradeSignal(std::move(result));
      }

      assert(bids.front().priceLevel < order.priceLevel);

      order_status_e status = order_status_e::partially_filled;
      while (!asks.empty()) {
        auto &ask = asks.front();
        if (ask.priceLevel <= order.priceLevel) { // full or partial
          double const price = std::max(ask.priceLevel, order.priceLevel);
          double const execQty = std::min(order.quantity, ask.totalQuantity);

          if ((order.quantity - execQty) == 0.0)
            status = order_status_e::filled;
          auto trade = getNewTrade(order, status, execQty, price);
          auto otherTrades = getExecutedTradesFromOrders(ask, execQty, price);

          ask.totalQuantity -= execQty;
          order.quantity -= execQty;

          if (ask.totalQuantity == 0.0)
            asks.erase(asks.begin());
          result.push_back(std::move(trade));
          result.insert(result.end(), otherTrades.begin(), otherTrades.end());

          if (order.quantity == 0.0)
            return broadcastTradeSignal(std::move(result));
        } else { // partial
          assert(qty > 0.0);
          break;
        }
      }

      assert(qty > 0.0);

      if (qty > 0.0) {
        updateSidesWithNewOrder(order, bids, greaterComparator);
        return broadcastTradeSignal(std::move(result));
      }
      // something is wrong with the order matching engine implementation
      throw std::runtime_error("internal error");
    } else if (order.side == trade_side_e::sell) { // selling
      if (bids.empty() || (order.priceLevel > bids.front().priceLevel)) {
        updateSidesWithNewOrder(order, bids, greaterComparator);
        return broadcastTradeSignal(std::move(result));
      }

      assert(!(asks.empty() && bids.empty()));

      auto const orderID = getOrderNumber();
      order_status_e status = order_status_e::partially_filled;

      while (!bids.empty()) {
        auto &bid = bids.front();
        if (bids.front().priceLevel >= order.priceLevel) {
          double const price = std::min(bid.priceLevel, order.priceLevel);
          double const execQty = std::min(order.quantity, bid.totalQuantity);

          if ((order.quantity - execQty) == 0.0)
            status = order_status_e::filled;
          auto trade = getNewTrade(order, status, execQty, price);
          auto otherTrades = getExecutedTradesFromOrders(bid, execQty, price);
          order.quantity -= execQty;
          bid.totalQuantity -= execQty;

          if (bid.totalQuantity == 0.0)
            bids.erase(bids.begin());
          result.push_back(std::move(trade));
          result.insert(result.end(), otherTrades.begin(), otherTrades.end());

          if (order.quantity == 0.0)
            return broadcastTradeSignal(std::move(result));
        } else {
          break;
        }
      }

      assert(order.quantity > 0.0);

      if (order.quantity > 0.0) {
        updateSidesWithNewOrder(order, asks, lesserComparator);
        return broadcastTradeSignal(std::move(result));
      }
      // something is wrong with the order matching engine implementation
      throw std::runtime_error("internal error");
    } else { // cancel
      //
    }
  } // end of limit order

  return broadcastTradeSignal(std::move(result));
}

trade_list_t
spot_order_book_t::getExecutedTradesFromOrders(details::order_meta_data_t &data,
                                               double quantityTraded,
                                               double const priceLevel) {
  trade_list_t trades;
  while (quantityTraded > 0.0) {
    if (data.orders.empty())
      break;
    auto &order = data.orders.front();
    double const tempQty = std::min(order.quantity, quantityTraded);
    order_status_e status = order_status_e::partially_filled;

    if ((order.quantity - tempQty) == 0.0)
      status = order_status_e::filled;
    auto trade = getNewTrade(order, status, tempQty, priceLevel);
    order.quantity -= tempQty;
    quantityTraded -= tempQty;
    if (order.quantity == 0.0)
      data.orders.erase(data.orders.begin());
    trades.push_back(std::move(trade));
  }
  return trades;
}

trade_data_t spot_order_book_t::getNewTrade(order_data_t const &order,
                                            order_status_e const status,
                                            double const qty,
                                            double const amount) {
  static int64_t tradeNumber = 0x0'000'320'012;

  trade_data_t trade;
  trade.quantityExecuted = qty;
  trade.amountPerPiece = amount;
  trade.orderID = order.orderID;
  trade.side = order.side;
  trade.tokenName = m_symbol->name;
  trade.tradeID = tradeNumber++;
  trade.eventTime = std::time(nullptr);
  trade.tradeType = order.type;
  trade.status = status;

  if (order.user) // send notification to the "orderer"
    order.user->OnNewTrade(trade);
  return trade;
}

trade_list_t
spot_order_book_t::marketMatcher(std::vector<details::order_meta_data_t> &list,
                                 double &amountAvailableToSpend,
                                 order_data_t const &order) {
  trade_list_t result;
  if (list.empty())
    return result;

  order_status_e status = order_status_e::partially_filled;
  while (amountAvailableToSpend > 0.0 && !list.empty()) {
    auto &front = list.front();
    double const price = front.priceLevel;
    double const expectedExecQty = price / amountAvailableToSpend;
    double const execQty = std::min(expectedExecQty, front.totalQuantity);
    double const amountSpent = execQty * price;

    if ((front.totalQuantity - execQty) == 0.0)
      status = order_status_e::filled;
    auto trade = getNewTrade(order, status, execQty, price);
    auto otherTrades = getExecutedTradesFromOrders(front, execQty, price);
    front.totalQuantity -= execQty;
    amountAvailableToSpend -= amountSpent;

    if (front.totalQuantity == 0.0)
      list.erase(list.begin());

    result.push_back(trade);
    result.insert(result.end(), otherTrades.begin(), otherTrades.end());
  }
  return result;
}

/*
Asks
1.3 -> 100
1.29 -> 98
1.28 -> 3
1.26 -> 12
1.21 -> 15
1.19 -> 10
1.17 -> 11
============= 8 <= BUY (LIMIT)
1.19 -> 3
1.18 -> 1
1.17 -> 5
1.14 -> 11
1.12 -> 13
1.10 -> 52
Bids
*/

void spot_order_book_t::shakeOrderBook() {
  auto &bids = m_orderBook.bids;
  auto &asks = m_orderBook.asks;

  if (asks.empty() || bids.empty() ||
      bids.front().priceLevel < asks.front().priceLevel)
    return;

  trade_list_t result{};
  while (!(asks.empty() && bids.empty())) {
    auto &ask = asks.front();
    auto &bid = bids.front();
    if (ask.priceLevel <= bid.priceLevel) {
      double const price = std::max(ask.priceLevel, bid.priceLevel);
      double const execQty = std::min(bid.totalQuantity, ask.totalQuantity);

      auto otherTrades = getExecutedTradesFromOrders(bid, execQty, price);
      result.insert(result.end(), otherTrades.begin(), otherTrades.end());
      otherTrades = getExecutedTradesFromOrders(ask, execQty, price);
      result.insert(result.end(), otherTrades.begin(), otherTrades.end());

      ask.totalQuantity -= execQty;
      if (ask.totalQuantity == 0.0)
        asks.erase(asks.begin());

      bid.totalQuantity -= execQty;
      if (bid.totalQuantity == 0.0)
        bids.erase(bids.begin());
    } else { // nothing to be done again
      break;
    }
  }

  NewTradesCreated(std::move(result));
}

#ifdef _DEBUG
void spot_order_book_t::printOrderBook() {
  for (auto const &ask : m_orderBook.asks)
    spdlog::debug("{} -> {}", ask.priceLevel, ask.totalQuantity);

  spdlog::debug("================");

  for (auto const &bid : m_orderBook.bids)
    spdlog::debug("{} -> {}", bid.priceLevel, bid.totalQuantity);
  spdlog::debug("<<========END========>>\n");
}
#endif

} // namespace backtesting
