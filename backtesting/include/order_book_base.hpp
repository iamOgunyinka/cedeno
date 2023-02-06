#pragma once

#include <boost/asio/io_context.hpp>
#include <memory>

#include "Signals/Signal.h"
#include "depth_data.hpp"
#include "user_data.hpp"

namespace net = boost::asio;

namespace backtesting {
class order_book_t;
}

namespace matching_engine {
void matchOrder(backtesting::order_book_t &orderBook,
                backtesting::order_data_t const &order);
void cancelOrder(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order);
} // namespace matching_engine

namespace backtesting {
class order_book_t {
public:
  friend void
  matching_engine::matchOrder(backtesting::order_book_t &orderBook,
                              backtesting::order_data_t const &order);
  friend void
  matching_engine::cancelOrder(backtesting::order_book_t &orderBook,
                               backtesting::order_data_t const &order);

  order_book_t(net::io_context &ioContext,
               data_streamer_t<depth_data_t> dataStreamer,
               internal_token_data_t *symbol, trade_type_e const tradeType)
      : m_ioContext(ioContext), m_dataStreamer(std::move(dataStreamer)),
        m_symbol(symbol), m_tradeType(tradeType) {}

  virtual ~order_book_t() {}
  virtual void run() = 0;
  Gallant::Signal1<trade_list_t> NewTradesCreated;

protected:
  virtual void match(backtesting::order_data_t order) = 0;
  virtual void cancel(backtesting::order_data_t order) = 0;

  net::io_context &m_ioContext;
  data_streamer_t<depth_data_t> m_dataStreamer;
  internal_token_data_t *m_symbol = nullptr;
  trade_type_e m_tradeType;
};
} // namespace backtesting
