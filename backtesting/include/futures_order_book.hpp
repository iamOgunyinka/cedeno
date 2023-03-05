#pragma once

#include "order_book_base.hpp"

namespace backtesting {

class futures_order_book_t : public order_book_base_t {
public:
  futures_order_book_t(net::io_context &ioContext,
                       data_streamer_t<depth_data_t> dataStream,
                       internal_token_data_t *token);
  ~futures_order_book_t() {}

private:
#ifdef _DEBUG
  void printOrderBook() override {}
#endif
};
} // namespace backtesting
