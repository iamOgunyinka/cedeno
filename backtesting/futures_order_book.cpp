#include "futures_order_book.hpp"

namespace backtesting {
futures_order_book_t::futures_order_book_t(net::io_context &ioContext,
                                           data_streamer_t<depth_data_t> stream,
                                           internal_token_data_t *symbol)
    : order_book_t(ioContext, std::move(stream), symbol,
                   trade_type_e::futures) {}
futures_order_book_t::~futures_order_book_t() {}
void futures_order_book_t::run() {}

void futures_order_book_t::match(backtesting::order_data_t order) {
  (void)order;
}
void futures_order_book_t::cancel(backtesting::order_data_t order) {
  (void)order;
}
} // namespace backtesting
