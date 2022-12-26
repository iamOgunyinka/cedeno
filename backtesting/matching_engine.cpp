#include "matching_engine.hpp"
#include "order_book.hpp"

namespace matching_engine {

backtesting::trade_list_t match_order(backtesting::order_book_t &orderBook,
                                      backtesting::order_data_t const &order) {
  return orderBook.match(order);
}

} // namespace matching_engine
