#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <memory>

#include "depth_data.hpp"
#include "user_data.hpp"

namespace net = boost::asio;

namespace backtesting {
class order_book_t;
}

namespace matching_engine {
backtesting::trade_list_t match_order(backtesting::order_book_t &orderBook,
                                      backtesting::order_data_t const &order);
}

namespace backtesting {

class order_book_t {
public:
  static friend trade_list_t
  matching_engine::match_order(order_book_t &orderBook,
                               order_data_t const &order);
  order_book_t(net::io_context &ioContext,
               data_streamer_t<depth_data_t> dataStreamer);
  ~order_book_t();
  void run();

private:
#ifdef _DEBUG
  void printOrderBook();
#endif

  [[nodiscard]] trade_list_t match(order_data_t order);
  void setNextTimer();
  [[nodiscard]] trade_list_t shakeOrderBook();
  void updateOrderBook(depth_data_t &&newestData);
  net::io_context &m_ioContext;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  data_streamer_t<depth_data_t> m_dataStreamer;
  depth_data_t m_currentBook;
  depth_data_t m_nextData;
  time_t m_currentTimer;
};

} // namespace backtesting
