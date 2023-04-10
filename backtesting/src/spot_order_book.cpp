#include "spot_order_book.hpp"
#include "log_info.hpp"
#include <mutex>

extern bool verbose;

namespace backtesting {

spot_order_book_t::spot_order_book_t(
    net::io_context &ioContext, data_streamer_t<depth_data_t> depthStreamer,
    std::optional<data_streamer_t<reader_trade_data_t>> tradeStreamer,
    internal_token_data_t *symbol)
    : order_book_base_t(ioContext, std::move(depthStreamer),
                        std::move(tradeStreamer), symbol) {}

#ifdef _DEBUG
void spot_order_book_t::printOrderBook() {
  for (auto const &ask : m_orderBook.asks) {
    PRINT_INFO("{} -> {}", ask.priceLevel, ask.totalQuantity);
  }

  PRINT_INFO("================");

  for (auto const &bid : m_orderBook.bids) {
    PRINT_INFO("{} -> {}", bid.priceLevel, bid.totalQuantity);
  }
  PRINT_INFO("<<========END========>>\n");
}
#endif

trade_list_t spot_order_book_t::marketMatcherImpl(
    std::vector<details::order_book_entry_t> &list,
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
    auto trade = getNewTrade(order, status, execQty, price, true);
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
