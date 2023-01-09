#pragma once

#include "user_data.hpp"

namespace backtesting {
class order_book_t;
}

namespace matching_engine {
backtesting::trade_list_t
match_order(backtesting::order_book_t &orderBook,
            backtesting::order_data_t const &order);
}
