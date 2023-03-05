#include "spot_order_book.hpp"
#include <mutex>

#ifdef _DEBUG
#include <spdlog/spdlog.h>
#endif

namespace backtesting {

spot_order_book_t::spot_order_book_t(net::io_context &ioContext,
                                     data_streamer_t<depth_data_t> dataStreamer,
                                     internal_token_data_t *symbol)
    : order_book_base_t(ioContext, std::move(dataStreamer), symbol,
                        trade_type_e::spot) {}

#ifdef _DEBUG
void spot_order_book_t::printOrderBook() {
  for (auto const &ask : m_orderBook.asks)
    spdlog::debug("{} -> {}", ask.priceLevel, ask.totalQuantity);

  spdlog::debug("================");

  for (auto const &bid : m_orderBook.bids)
    spdlog::debug("{} -> {}", bid.priceLevel, bid.totalQuantity);
  spdlog::debug("<<========END========>>\n");
}
#endif

} // namespace backtesting
