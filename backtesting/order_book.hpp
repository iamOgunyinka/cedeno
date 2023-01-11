#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <memory>

#include "Signals/Signal.h"
#include "depth_data.hpp"
#include "user_data.hpp"

namespace net = boost::asio;

namespace backtesting {
class order_book_t;
}

namespace matching_engine {
void match_order(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order);
}

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
} // namespace details

class order_book_t {
public:
  friend void
  matching_engine::match_order(order_book_t &orderBook,
                               backtesting::order_data_t const &order);
  Gallant::Signal1<trade_list_t> NewTradesCreated;
  order_book_t(net::io_context &ioContext,
               data_streamer_t<depth_data_t> dataStreamer,
               trade_type_e const t);
  ~order_book_t();
  void run();

private:
#ifdef _DEBUG
  void printOrderBook();
#endif

  void match(backtesting::order_data_t order);
  void setNextTimer();
  void shakeOrderBook();
  void updateOrderBook(depth_data_t &&newestData);

private:
  net::io_context &m_ioContext;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  data_streamer_t<depth_data_t> m_dataStreamer;
  details::order_book_meta_t m_orderBook;
  depth_data_t m_nextData;
  time_t m_currentTimer;
  trade_type_e const m_tradeType;
};

details::order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       trade_side_e const side, trade_type_e const tradeType);
} // namespace backtesting
