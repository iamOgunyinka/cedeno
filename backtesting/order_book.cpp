#include "order_book.hpp"

namespace backtesting {

static int64_t orderNumber = 0x2'300'820'000;
static int64_t tradeNumber = 0x1'500'320'012;

bool isGreater(depth_data_t::depth_meta_t const &a,
               depth_data_t::depth_meta_t const &b) {
  return a.priceLevel > b.priceLevel;
}

bool isLesser(depth_data_t::depth_meta_t const &a,
              depth_data_t::depth_meta_t const &b) {
  return a.priceLevel < b.priceLevel;
}

struct lesser_comparator_t {
  bool operator()(double const a, depth_data_t::depth_meta_t const &b) const {
    return a < b.priceLevel;
  }
  bool operator()(depth_data_t::depth_meta_t const &a, double const b) const {
    return a.priceLevel < b;
  }
};

struct greater_comparator_t {
  bool operator()(double const a, depth_data_t::depth_meta_t const &b) const {
    return a > b.priceLevel;
  }
  bool operator()(depth_data_t::depth_meta_t const &a, double const b) const {
    return a.priceLevel > b;
  }
};

static lesser_comparator_t lesserComparator{};
static greater_comparator_t greaterComparator{};

template <typename Func>
void updateSidesWithNewData(std::vector<depth_data_t::depth_meta_t> const &src,
                            std::vector<depth_data_t::depth_meta_t> &dest,
                            Func comparator) {
  for (auto const &d : src) {
    auto iter =
        std::lower_bound(dest.begin(), dest.end(), d.priceLevel, comparator);
    if (iter != dest.end() && iter->priceLevel == d.priceLevel) {
      if (d.quantity == 0.0) {
        dest.erase(iter);
        continue;
      } else {
        iter->quantity += d.quantity;
      }
    } else {
      dest.insert(iter, d);
    }
  }
  dest.erase(std::remove_if(dest.begin(), dest.end(),
                            [](auto const &a) { return a.quantity == 0.0; }),
             dest.end());
}

void pushTrade(trade_list_t &result, user_order_request_t const &order,
               double const qty, double const amount, uint64_t const orderID) {
  trade_list_t::value_type trade;
  trade.quantityExec = qty;
  trade.amountPerPiece = amount;
  trade.orderID = orderID;
  trade.side = order.side;
  trade.token.name = order.token.name;
  trade.tradeID = tradeNumber++;
  result.push_back(std::move(trade));
};

trade_list_t marketMatcher(std::vector<depth_data_t::depth_meta_t> &list,
                           user_order_request_t const &order, double quantity,
                           uint64_t const orderID) {
  if (list.empty())
    return {};
  trade_list_t result;

  while (quantity > 0.0 && !list.empty()) {
    auto &front = list.front();
    if (front.quantity > quantity) {
      pushTrade(result, order, quantity, front.priceLevel, orderID);
      front.quantity -= quantity;
      quantity = 0.0;
    } else if (front.quantity == quantity) {
      pushTrade(result, order, front.quantity, front.priceLevel, orderID);
      quantity = 0.0;
      list.erase(list.begin());
    } else {
      pushTrade(result, order, front.quantity, front.priceLevel, orderID);
      quantity -= front.quantity;
      list.erase(list.begin());
    }
  }
  return result;
}

order_book_t::order_book_t(net::io_context &ioContext,
                           data_streamer_t<depth_data_t> dataStreamer)
    : m_ioContext(ioContext), m_dataStreamer(std::move(dataStreamer)),
      m_currentBook(m_dataStreamer.getNextData()),
      m_currentTimer(m_currentBook.eventTime) {
  std::sort(m_currentBook.asks.begin(), m_currentBook.asks.end(), isLesser);
  std::sort(m_currentBook.bids.begin(), m_currentBook.bids.end(), isGreater);
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

trade_list_t order_book_t::match(user_order_request_t order) {
  auto const isBuying = (order.side == trade_side_e::buy);
  auto &bids = m_currentBook.bids;
  auto &asks = m_currentBook.asks;

  trade_list_t result;
  if (order.market == trade_market_e::market) {
    return marketMatcher(isBuying ? asks : bids, order, order.quantity,
                         orderNumber++);
  } else if (order.market == trade_market_e::limit) {
    if (isBuying) {
      double &qty = order.quantity;
      if (asks.empty() ||
          (!bids.empty() && order.priceLevel <= bids.front().priceLevel) ||
          (!asks.empty() && asks.front().priceLevel > order.priceLevel)) {
        depth_data_t::depth_meta_t temp{order.priceLevel, qty};
        updateSidesWithNewData({std::move(temp)}, bids, greaterComparator);
        pushTrade(result, order, 0.0, 0.0, orderNumber++);
        return result;
      }

      assert(bids.front().priceLevel < order.priceLevel);

      auto const orderID = orderNumber++;
      while (!asks.empty()) {
        if (asks.front().priceLevel <= order.priceLevel) { // full or partial
          if (qty <= asks.front().quantity) {
            pushTrade(result, order, qty, asks.front().priceLevel, orderID);
            asks.front().quantity -= qty;
            qty = 0.0;
            if (asks.front().quantity == 0.0)
              asks.erase(asks.begin());
            return result;
          } else if (qty > asks.front().quantity) {
            pushTrade(result, order, asks.front().quantity,
                      asks.front().priceLevel, orderID);
            qty -= asks.front().quantity;
            asks.erase(asks.begin());
          }
        } else { // partial
          assert(qty > 0.0);
          break;
        }
      }

      assert(qty > 0.0);

      if (qty > 0.0) {
        depth_data_t::depth_meta_t temp{order.priceLevel, qty};
        updateSidesWithNewData({std::move(temp)}, bids, greaterComparator);
        return result;
      }
      // something is wrong with the order matching engine implementation
      throw std::runtime_error("internal error");
    } else if (order.side == trade_side_e::sell) { // selling
      if (bids.empty() || (order.priceLevel > bids.front().priceLevel)) {
        depth_data_t::depth_meta_t temp{order.priceLevel, order.quantity};
        updateSidesWithNewData({std::move(temp)}, bids, greaterComparator);
        pushTrade(result, order, 0.0, 0.0, orderNumber++);
        return result;
      }

      assert(!(asks.empty() && bids.empty()));

      auto const orderID = orderNumber++;
      while (!bids.empty()) {
        if (bids.front().priceLevel >= order.priceLevel) {
          if (order.quantity <= bids.front().quantity) {
            pushTrade(result, order, order.quantity, bids.front().priceLevel,
                      orderID);
            bids.front().quantity -= order.quantity;
            order.quantity = 0.0;
            if (bids.front().quantity == 0.0)
              bids.erase(bids.begin());
            return result;
          } else if (order.quantity > bids.front().quantity) {
            pushTrade(result, order, bids.front().quantity,
                      bids.front().priceLevel, orderID);
            order.quantity -= bids.front().quantity;
            bids.erase(bids.begin());
          }
        } else {
          break;
        }
      }

      assert(order.quantity > 0.0);

      if (order.quantity > 0.0) {
        depth_data_t::depth_meta_t temp{order.priceLevel, order.quantity};
        updateSidesWithNewData({std::move(temp)}, asks, lesserComparator);
        return result;
      }
      // something is wrong with the order matching engine implementation
      throw std::runtime_error("internal error");
    } else { // cancel
      //
    }
  } // end of limit order

  return result;
}

trade_list_t order_book_t::shakeOrderBook() {
  auto &bids = m_currentBook.bids;
  auto &asks = m_currentBook.asks;

  trade_list_t result;
  // buying first
  if (asks.empty() || bids.empty() ||
      bids.front().priceLevel < asks.front().priceLevel)
    return result;

  while (!asks.empty()) {
    if (bids.empty())
      break;
    if (asks.front().priceLevel <= bids.front().priceLevel) {
      if (bids.front().quantity <= asks.front().quantity) {
        asks.front().quantity -= bids.front().quantity;
        bids.erase(bids.begin());

        if (asks.front().quantity == 0.0)
          asks.erase(asks.begin());
      } else if (bids.front().quantity >= asks.front().quantity) {
        bids.front().quantity -= asks.front().quantity;
        asks.erase(asks.begin());
        if (bids.front().quantity == 0.0)
          bids.erase(bids.begin());
      }
    } else { // nothing to be done again
      break;
    }
  }
  return result;
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
  std::cout << "TimeDiff: " << timeDiff << std::endl;
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
  for (auto const &ask : m_currentBook.asks)
    std::cout << ask.priceLevel << " -> " << ask.quantity << std::endl;
  std::cout << "================" << std::endl;
  for (auto const &bid : m_currentBook.bids)
    std::cout << bid.priceLevel << " -> " << bid.quantity << std::endl;
  std::cout << "<<========END========>>" << std::endl << std::endl;
}
#endif

void order_book_t::updateOrderBook(depth_data_t &&newestData) {
  updateSidesWithNewData(newestData.asks, m_currentBook.asks, lesserComparator);
  updateSidesWithNewData(newestData.bids, m_currentBook.bids,
                         greaterComparator);
#ifdef _DEBUG
  // printOrderBook();
#endif // _DEBUG
}
} // namespace backtesting
