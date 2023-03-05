#include "futures_order_book.hpp"

namespace backtesting {

futures_order_book_t::futures_order_book_t(
    net::io_context &ioContext, data_streamer_t<depth_data_t> dataStream,
    internal_token_data_t *token)
    : order_book_base_t(ioContext, std::move(dataStream), token,
                        trade_type_e::futures) {}
} // namespace backtesting
