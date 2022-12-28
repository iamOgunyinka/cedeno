#include "user_data.hpp"

namespace backtesting {

order_data_t user_data_t::createOrder(std::string const &tokenName,
                                      double const quantity, double const price,
                                      double const leverage,
                                      trade_type_e const type,
                                      trade_side_e const side,
                                      trade_market_e const market) {
  order_data_t order;
  order.leverage = leverage;
  order.market = market;
  order.orderID = 0;
  order.priceLevel = price;
  order.quantity = quantity;
  order.side = side;
  order.tokenName = tokenName;
  order.type = type;
  order.userID = this->userID;
  return order;
}
} // namespace backtesting
