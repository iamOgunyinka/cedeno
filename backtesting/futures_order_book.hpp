#pragma once

#include "order_book_base.hpp"

namespace backtesting {
class futures_order_book_t : public order_book_t {
public:
  futures_order_book_t(net::io_context &, data_streamer_t<depth_data_t>,
                       internal_token_data_t *);
  ~futures_order_book_t();
  void run() override;

private:
  void match(backtesting::order_data_t order) override;
  void cancel(backtesting::order_data_t order) override;
};
} // namespace backtesting
