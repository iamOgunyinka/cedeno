#include "futures_order_book.hpp"

namespace backtesting {

futures_order_book_t::futures_order_book_t(
    net::io_context &ioContext, data_streamer_t<depth_data_t> dataStream,
    internal_token_data_t *token)
    : order_book_base_t(ioContext, std::move(dataStream), token) {}

trade_list_t futures_order_book_t::marketMatcherImpl(
    std::vector<details::order_book_entry_t> &list, double &,
    order_data_t const &order) {
  trade_list_t result;
  order_status_e status = order_status_e::partially_filled;
  double quantity = order.quantity;
  while (quantity > 0.0 && !list.empty()) {
    auto &front = list.front();
    double const price = front.priceLevel;
    double const execQty = (std::min)(quantity, front.totalQuantity);

    if ((front.totalQuantity - execQty) == 0.0)
      status = order_status_e::filled;
    auto trade = getNewTrade(order, status, execQty, price);
    auto otherTrades = getExecutedTradesFromOrders(front, execQty, price);

    // broadcast current market price of symbol
    NewMarketPrice(m_symbol, price);

    front.totalQuantity -= execQty;
    quantity -= execQty;

    if (front.totalQuantity == 0.0)
      list.erase(list.begin());

    result.push_back(trade);
    result.insert(result.end(), otherTrades.begin(), otherTrades.end());
  }
  return result;
}
} // namespace backtesting
