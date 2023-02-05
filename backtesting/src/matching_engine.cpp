#include "matching_engine.hpp"
#include "spot_order_book.hpp"

namespace matching_engine {

void matchOrder(backtesting::order_book_t &orderBook,
                backtesting::order_data_t const &order) {
  return orderBook.match(order);
}

void cancelOrder(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order) {
  return orderBook.cancel(order);
}
} // namespace matching_engine
