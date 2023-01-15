#include "order_book.hpp"
#include <spdlog/spdlog.h>

namespace backtesting {
using simple_sort_callback_t = bool (*)(details::order_meta_data_t const &,
                                        details::order_meta_data_t const &);

static int64_t orderNumber = 0x0'300'820'000;

bool isGreater(details::order_meta_data_t const &a,
               details::order_meta_data_t const &b) {
  return a.priceLevel > b.priceLevel;
}

bool isLesser(details::order_meta_data_t const &a,
              details::order_meta_data_t const &b) {
  return a.priceLevel < b.priceLevel;
}

struct lesser_comparator_t {
  bool operator()(double const a, details::order_meta_data_t const &b) const {
    return a < b.priceLevel;
  }

  bool operator()(details::order_meta_data_t const &a, double const b) const {
    return a.priceLevel < b;
  }
};
static lesser_comparator_t lesserComparator{};

struct greater_comparator_t {
  bool operator()(double const a, details::order_meta_data_t const &b) const {
    return a > b.priceLevel;
  }

  bool operator()(details::order_meta_data_t const &a, double const b) const {
    return a.priceLevel > b;
  }
};
static greater_comparator_t greaterComparator{};

template <typename Comparator>
void updateSidesWithNewOrder(order_data_t const &order,
                             std::vector<details::order_meta_data_t> &dest,
                             Comparator comparator) {
  auto iter =
      std::lower_bound(dest.begin(), dest.end(), order.priceLevel, comparator);
  if (iter != dest.end() && iter->priceLevel == order.priceLevel) {
    if (order.quantity == 0.0)
      dest.erase(iter);
    else {
      iter->totalQuantity += order.quantity;
      iter->orders.push_back(order);
    }
  } else {
    details::order_meta_data_t newInsert{};
    newInsert.orders.push_back(std::move(order));
    newInsert.priceLevel = order.priceLevel;
    newInsert.totalQuantity = order.quantity;
    dest.insert(iter, std::move(newInsert));
  }
}

template <typename Func>
void updateSidesWithNewOrder(order_list_t const &src,
                             std::vector<details::order_meta_data_t> &dest,
                             Func comparator) {
  for (auto const &d : src)
    updateSidesWithNewOrder(d, dest, comparator);

  dest.erase(std::remove_if(dest.begin(), dest.end(),
                            [](auto const &a) { return a.quantity == 0.0; }),
             dest.end());
}

template <typename Comparator>
void updateSidesWithNewDepth(std::vector<depth_data_t::depth_meta_t> const &src,
                             std::vector<details::order_meta_data_t> &dest,
                             trade_side_e const side,
                             trade_type_e const tradeType,
                             Comparator comparator) {
  for (auto const &d : src) {
    auto iter =
        std::lower_bound(dest.begin(), dest.end(), d.priceLevel, comparator);
    if (iter != dest.end() && iter->priceLevel == d.priceLevel) {
      if (d.quantity == 0.0) {
        dest.erase(iter);
        continue;
      } else {
        iter->totalQuantity += d.quantity;
        order_data_t order;
        order.market = market_type_e::limit;
        order.priceLevel = d.priceLevel;
        order.quantity = d.quantity;
        order.side = side;
        order.type = tradeType;
        order.orderID = orderNumber++;
        iter->orders.push_back(std::move(order));
      }
    } else {
      dest.insert(iter, orderMetaDataFromDepth(d, side, tradeType));
    }
  }

  dest.erase(std::remove_if(dest.begin(), dest.end(),
                            [](details::order_meta_data_t const &a) {
                              return a.totalQuantity == 0.0;
                            }),
             dest.end());
}

[[nodiscard]] trade_data_t getNewTrade(order_data_t const &order,
                                       double const qty, double const amount) {
  static int64_t tradeNumber = 0x0'500'320'012;

  trade_data_t trade;
  trade.quantityExecuted = qty;
  trade.amountPerPiece = amount;
  trade.orderID = order.orderID;
  trade.side = order.side;
  trade.tokenName = order.tokenName;
  trade.tradeID = tradeNumber++;
  trade.eventTime = std::time(nullptr);
  trade.tradeType = order.type;

  if (order.user) // send notification to the "orderer"
    order.user->OnNewTrade(order, qty, amount * qty);
  return trade;
}

[[nodiscard]] trade_list_t
getExecutedTradesFromOrders(details::order_meta_data_t &data,
                            double quantityTraded, double const priceLevel) {
  trade_list_t trades;
  while (quantityTraded > 0.0) {
    if (data.orders.empty())
      break;
    auto &order = data.orders.front();
    double const tempQty = std::min(order.quantity, quantityTraded);
    auto trade = getNewTrade(order, tempQty, priceLevel);
    order.quantity -= tempQty;
    quantityTraded -= tempQty;
    if (order.quantity == 0.0)
      data.orders.erase(data.orders.begin());
    trades.push_back(std::move(trade));
  }
  return trades;
}

trade_list_t marketMatcher(std::vector<details::order_meta_data_t> &list,
                           double &amountAvailableToSpend,
                           order_data_t const &order) {
  trade_list_t result;
  if (list.empty())
    return result;

  while (amountAvailableToSpend > 0.0 && !list.empty()) {
    auto &front = list.front();
    double const price = front.priceLevel;
    double const expectedExecQty = price / amountAvailableToSpend;
    double const execQty = std::min(expectedExecQty, front.totalQuantity);
    double const amountSpent = execQty * price;

    auto trade = getNewTrade(order, execQty, price);
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

details::order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       trade_side_e const side, trade_type_e const tradeType) {
  details::order_meta_data_t d;
  d.totalQuantity += depth.quantity;
  d.priceLevel = depth.priceLevel;

  order_data_t order;
  order.market = market_type_e::limit;
  order.priceLevel = depth.priceLevel;
  order.quantity = depth.quantity;
  order.side = side;
  order.type = tradeType;
  order.orderID = orderNumber++;
  d.orders.push_back(order);
  return d;
}

void insertAndSort(std::vector<depth_data_t::depth_meta_t> const &depthList,
                   std::vector<details::order_meta_data_t> &dst,
                   trade_side_e const side, trade_type_e const tradeType,
                   simple_sort_callback_t comparator) {
  for (auto const &depth : depthList)
    dst.push_back(orderMetaDataFromDepth(depth, side, tradeType));

  std::sort(dst.begin(), dst.end(), comparator);
}

order_book_t::order_book_t(net::io_context &ioContext,
                           data_streamer_t<depth_data_t> dataStreamer,
                           trade_type_e const tradeType)
    : m_ioContext(ioContext), m_dataStreamer(std::move(dataStreamer)),
      m_currentTimer(0), m_tradeType(tradeType) {
  auto firstData = m_dataStreamer.getNextData();
  insertAndSort(firstData.bids, m_orderBook.bids, trade_side_e::buy,
                m_tradeType, isGreater);
  insertAndSort(firstData.asks, m_orderBook.asks, trade_side_e::sell,
                m_tradeType, isLesser);
  m_currentTimer = firstData.eventTime;
}

order_book_t::~order_book_t() {
  if (m_periodicTimer) {
    m_periodicTimer->cancel();
    m_periodicTimer.reset();
  }
}

void order_book_t::run() {
  if (m_periodicTimer) // already running
    return;

  setNextTimer();
}

void order_book_t::match(order_data_t order) {
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

      while (!asks.empty()) {
        auto &ask = asks.front();
        if (ask.priceLevel <= order.priceLevel) { // full or partial
          double const price = std::max(ask.priceLevel, order.priceLevel);
          double const execQty = std::min(order.quantity, ask.totalQuantity);

          auto trade = getNewTrade(order, execQty, price);
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

      auto const orderID = orderNumber++;
      while (!bids.empty()) {
        auto &bid = bids.front();
        if (bids.front().priceLevel >= order.priceLevel) {
          double const price = std::min(bid.priceLevel, order.priceLevel);
          double const execQty = std::min(order.quantity, bid.totalQuantity);

          auto trade = getNewTrade(order, execQty, price);
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

void order_book_t::shakeOrderBook() {
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

void order_book_t::setNextTimer() {
  if (!m_periodicTimer)
    m_periodicTimer.reset(new net::deadline_timer(m_ioContext));

  m_nextData = m_dataStreamer.getNextData();

  // no more data but we need to keep the simulator running
  if (m_nextData.asks.empty() && m_nextData.bids.empty())
    m_nextData.eventTime = m_currentTimer + 1'000;

  auto const timeDiff = m_nextData.eventTime - m_currentTimer;
#ifdef _DEBUG
  spdlog::debug("TimeDiff: {}", timeDiff);
  assert(timeDiff > 0);
#else
  if (timeDiff <= 0LL)
    return;
#endif // _DEBUG

  m_currentTimer = m_nextData.eventTime;

  m_periodicTimer->expires_from_now(boost::posix_time::milliseconds(timeDiff));
  m_periodicTimer->async_wait([this](boost::system::error_code const ec) {
    updateOrderBook(std::move(m_nextData));
    setNextTimer();
  });
}

#ifdef _DEBUG
void order_book_t::printOrderBook() {
  for (auto const &ask : m_orderBook.asks)
    spdlog::debug("{} -> {}", ask.priceLevel, ask.totalQuantity);

  spdlog::debug("================");

  for (auto const &bid : m_orderBook.bids)
    spdlog::debug("{} -> {}", bid.priceLevel, bid.totalQuantity);
  spdlog::debug("<<========END========>>\n");
}
#endif

void order_book_t::updateOrderBook(depth_data_t &&newestData) {
  updateSidesWithNewDepth(newestData.asks, m_orderBook.asks, trade_side_e::sell,
                          m_tradeType, lesserComparator);
  updateSidesWithNewDepth(newestData.bids, m_orderBook.bids, trade_side_e::buy,
                          m_tradeType, greaterComparator);
  shakeOrderBook();
#ifdef _DEBUG
  printOrderBook();
#endif // _DEBUG
}
} // namespace backtesting
