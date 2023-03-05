#include "matching_engine.hpp"
#include "spot_order_book.hpp"

namespace matching_engine {

void placeOrder(backtesting::order_book_base_t &orderBook,
                backtesting::order_data_t const &order) {
  return orderBook.placeOrder(order);
}

void cancelOrder(backtesting::order_book_base_t &orderBook,
                 backtesting::order_data_t const &order) {
  return orderBook.cancelOrder(order);
}
} // namespace matching_engine
