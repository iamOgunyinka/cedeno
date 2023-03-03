#pragma once

#include <memory>

#include <boost/asio/deadline_timer.hpp>

#include "Signals/Signal.h"
#include "depth_data.hpp"
#include "user_data.hpp"
#include <algorithm>

namespace boost {
namespace asio {
class io_context;
} // namespace asio
} // namespace boost

namespace net = boost::asio;

namespace backtesting {
class order_book_base_t;
}

namespace matching_engine {
void matchOrder(backtesting::order_book_base_t &orderBook,
                backtesting::order_data_t const &order);
void cancelOrder(backtesting::order_book_base_t &orderBook,
                 backtesting::order_data_t const &order);
} // namespace matching_engine

namespace backtesting {

namespace details {
struct order_meta_data_t {
  double totalQuantity = 0.0;
  double priceLevel = 0.0;
  std::vector<order_data_t> orders;
};

struct order_book_meta_t {
  std::vector<order_meta_data_t> bids;
  std::vector<order_meta_data_t> asks;
};

struct lesser_comparator_t {
  bool operator()(double const a, order_meta_data_t const &b) const {
    return a < b.priceLevel;
  }

  bool operator()(order_meta_data_t const &a, double const b) const {
    return a.priceLevel < b;
  }
};

struct greater_comparator_t {
  bool operator()(double const a, order_meta_data_t const &b) const {
    return a > b.priceLevel;
  }

  bool operator()(order_meta_data_t const &a, double const b) const {
    return a.priceLevel > b;
  }
};

} // namespace details

class order_book_base_t {
public:
  friend void
  matching_engine::matchOrder(backtesting::order_book_base_t &orderBook,
                              backtesting::order_data_t const &order);
  friend void
  matching_engine::cancelOrder(backtesting::order_book_base_t &orderBook,
                               backtesting::order_data_t const &order);

  order_book_base_t(net::io_context &ioContext,
                    data_streamer_t<depth_data_t> dataStreamer,
                    internal_token_data_t *symbol,
                    trade_type_e const tradeType);
  virtual ~order_book_base_t();
  void run();

  Gallant::Signal1<trade_list_t> NewTradesCreated;
  Gallant::Signal1<depth_data_t> NewDepthObtained;

private:
  void updateOrderBook(depth_data_t &&newestData);
  void setNextTimer();

protected:
  virtual void match(backtesting::order_data_t order) = 0;
  virtual void cancel(backtesting::order_data_t order) = 0;
  virtual void shakeOrderBook() = 0;

#ifdef _DEBUG
  virtual void printOrderBook() = 0;
#endif

  net::io_context &m_ioContext;
  data_streamer_t<depth_data_t> m_dataStreamer;
  internal_token_data_t *m_symbol = nullptr;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  details::order_book_meta_t m_orderBook;
  depth_data_t m_nextData;
  time_t m_currentTimer = 0;
  trade_type_e const m_tradeType;
};

details::order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       internal_token_data_t *token, trade_side_e const side,
                       trade_type_e const tradeType);
int64_t getOrderNumber();

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
                             internal_token_data_t *token,
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
        order.token = token;
        order.orderID = getOrderNumber();
        iter->orders.push_back(std::move(order));
      }
    } else {
      dest.insert(iter, orderMetaDataFromDepth(d, token, side, tradeType));
    }
  }

  dest.erase(std::remove_if(dest.begin(), dest.end(),
                            [](details::order_meta_data_t const &a) {
                              return a.totalQuantity == 0.0;
                            }),
             dest.end());
}

} // namespace backtesting
