#include "spot_order_book.hpp"
#include <mutex>

#ifdef _DEBUG
#include <spdlog/spdlog.h>
#endif

namespace backtesting {

spot_order_book_t::spot_order_book_t(net::io_context &ioContext,
                                     data_streamer_t<depth_data_t> dataStreamer,
                                     internal_token_data_t *symbol)
    : order_book_base_t(ioContext, std::move(dataStreamer), symbol) {}

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

trade_list_t spot_order_book_t::marketMatcherImpl(
    std::vector<details::order_meta_data_t> &list,
    double &amountAvailableToSpend, order_data_t const &order) {
  trade_list_t result;
  order_status_e status = order_status_e::partially_filled;
  while (amountAvailableToSpend > 0.0 && !list.empty()) {
    auto &front = list.front();
    double const price = front.priceLevel;
    double const expectedExecQty = price / amountAvailableToSpend;
    double const execQty = (std::min)(expectedExecQty, front.totalQuantity);
    double const amountSpent = execQty * price;

    if ((front.totalQuantity - execQty) == 0.0)
      status = order_status_e::filled;
    auto trade = getNewTrade(order, status, execQty, price);
    auto otherTrades = getExecutedTradesFromOrders(front, execQty, price);

    // broadcast current market price of symbol
    NewMarketPrice(m_symbol, price);

    front.totalQuantity -= execQty;
    amountAvailableToSpend -= amountSpent;

    if (front.totalQuantity == 0.0)
      list.erase(list.begin());

    result.push_back(trade);
    result.insert(result.end(), otherTrades.begin(), otherTrades.end());
  }
  return result;
}

} // namespace backtesting
