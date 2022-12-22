#include "matching_engine.hpp"
#include "order_book.hpp"

namespace matching_engine {

int64_t match_order(backtesting::order_book_t &orderBook,
                    backtesting::user_order_request_t const &order) {
  return orderBook.match(order);
}

} // namespace matching_engine
