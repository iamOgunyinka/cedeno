#include "futures_order_book.hpp"

namespace backtesting {

futures_order_book_t::futures_order_book_t(
    net::io_context &ioContext, data_streamer_t<depth_data_t> dataStream,
    internal_token_data_t *token, double const makerFee, double const takerFee)
    : order_book_base_t(ioContext, std::move(dataStream), token,
                        trade_type_e::futures),
      m_makerFee(makerFee), m_takerFee(takerFee) {}

void futures_order_book_t::match(backtesting::order_data_t order) {
  (void)order;
}
void futures_order_book_t::cancel(backtesting::order_data_t order) {
  (void)order;
}

void futures_order_book_t::shakeOrderBook() {
  //
}
} // namespace backtesting
