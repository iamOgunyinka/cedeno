#pragma once

#include "order_book_base.hpp"

namespace backtesting {

class spot_order_book_t : public order_book_base_t {
public:
  spot_order_book_t(net::io_context &ioContext,
                    data_streamer_t<depth_data_t> dataStreamer,
                    internal_token_data_t *symbol);
  ~spot_order_book_t() {}

private:
#ifdef _DEBUG
  void printOrderBook() override;
#endif
};
} // namespace backtesting
