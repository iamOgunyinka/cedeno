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
void placeOrder(backtesting::order_book_base_t &orderBook,
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
  matching_engine::placeOrder(backtesting::order_book_base_t &orderBook,
                              backtesting::order_data_t const &order);
  friend void
  matching_engine::cancelOrder(backtesting::order_book_base_t &orderBook,
                               backtesting::order_data_t const &order);

  order_book_base_t(net::io_context &ioContext,
                    data_streamer_t<depth_data_t> dataStreamer,
                    internal_token_data_t *symbol);
  virtual ~order_book_base_t();
  void run();

  double currentSellPrice();
  double currentBuyPrice();
  Gallant::Signal1<trade_list_t> NewTradesCreated;
  Gallant::Signal1<depth_data_t> NewDepthObtained;
  Gallant::Signal2<internal_token_data_t *, double> NewMarketPrice;

private:
  void updateOrderBook(depth_data_t &&newestData);
  void setNextTimer();
  void cancelOrder(backtesting::order_data_t order);
  void placeOrder(backtesting::order_data_t order);
  void shakeOrderBook();
  [[nodiscard]] virtual trade_list_t
  marketMatcher(std::vector<details::order_meta_data_t> &list,
                double &amountAvailableToSpend, order_data_t const &order);

protected:
#ifdef _DEBUG
  virtual void printOrderBook() = 0;

#endif

  [[nodiscard]] virtual trade_list_t
  marketMatcherImpl(std::vector<details::order_meta_data_t> &list,
                    double &amountAvailableToSpend,
                    order_data_t const &order) = 0;

  [[nodiscard]] trade_data_t getNewTrade(order_data_t const &order,
                                         order_status_e const, double const qty,
                                         double const amount);
  [[nodiscard]] trade_list_t
  getExecutedTradesFromOrders(details::order_meta_data_t &data,
                              double quantityTraded, double const priceLevel);
  net::io_context &m_ioContext;
  data_streamer_t<depth_data_t> m_dataStreamer;
  internal_token_data_t *m_symbol = nullptr;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  details::order_book_meta_t m_orderBook;
  depth_data_t m_nextData;
  time_t m_currentTimer = 0;
};

details::order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       internal_token_data_t *token, trade_side_e const side,
                       trade_type_e const tradeType);
int64_t getOrderNumber();

} // namespace backtesting
