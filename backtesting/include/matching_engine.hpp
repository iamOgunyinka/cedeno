#pragma once

#include "user_data.hpp"

namespace backtesting {
class order_book_t;
} // namespace backtesting

namespace matching_engine {
void matchOrder(backtesting::order_book_t &orderBook,
                backtesting::order_data_t const &order);
void cancelOrder(backtesting::order_book_t &orderBook,
                 backtesting::order_data_t const &order);

} // namespace matching_engine
