#pragma once

#include "order_book_base.hpp"

namespace backtesting {

class spot_order_book_t : public order_book_base_t {
public:
  spot_order_book_t(net::io_context &ioContext,
                    data_streamer_t<depth_data_t> dataStreamer,
                    internal_token_data_t *symbol);
  ~spot_order_book_t() {}

private:
#ifdef _DEBUG
  void printOrderBook() override;
#endif

  void match(backtesting::order_data_t order) override;
  void cancel(backtesting::order_data_t order) override;
  void shakeOrderBook() override;

  [[nodiscard]] trade_data_t getNewTrade(order_data_t const &order,
                                         order_status_e const, double const qty,
                                         double const amount);
  [[nodiscard]] trade_list_t
  getExecutedTradesFromOrders(details::order_meta_data_t &data,
                              double quantityTraded, double const priceLevel);
  [[nodiscard]] trade_list_t
  marketMatcher(std::vector<details::order_meta_data_t> &list,
                double &amountAvailableToSpend, order_data_t const &order);
};

details::order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       internal_token_data_t *token, trade_side_e const side,
                       trade_type_e const tradeType);
} // namespace backtesting
