#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <memory>

#include "depth_data.hpp"

namespace net = boost::asio;

namespace backtesting {
class order_book_t;
}

namespace matching_engine {
int64_t match_order(backtesting::order_book_t &orderBook,
                    backtesting::user_order_request_t const &order);
}

namespace backtesting {

class order_book_t {
public:
  static friend int64_t
  matching_engine::match_order(order_book_t &orderBook,
                               user_order_request_t const &order);
  order_book_t(net::io_context &ioContext,
               data_streamer_t<depth_data_t> dataStreamer);
  ~order_book_t();
  void run();

private:
#ifdef _DEBUG
  void printOrderBook();
#endif

  int64_t match(user_order_request_t const& order);
  void setNextTimer();
  void updateOrderBook(depth_data_t &&newestData);
  net::io_context &m_ioContext;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  data_streamer_t<depth_data_t> m_dataStreamer;
  depth_data_t m_currentBook;
  depth_data_t m_nextData;
  time_t m_currentTimer;
};

} // namespace backtesting
