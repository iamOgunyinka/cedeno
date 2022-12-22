#pragma once

#include "common.hpp"

namespace backtesting {
class order_book_t;
}

namespace matching_engine {
int64_t match_order(backtesting::order_book_t &orderBook,
                    backtesting::user_order_request_t const &order);
}
