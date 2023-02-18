#pragma once

#include <boost/asio/deadline_timer.hpp>

#include "order_book_base.hpp"

namespace backtesting {
namespace details {
struct spot_order_meta_data_t {
  double totalQuantity = 0.0;
  double priceLevel = 0.0;
  std::vector<order_data_t> orders;
};

struct spot_order_book_meta_t {
  std::vector<spot_order_meta_data_t> bids;
  std::vector<spot_order_meta_data_t> asks;
};
} // namespace details

class spot_order_book_t : public order_book_t {
public:
  spot_order_book_t(net::io_context &ioContext,
                    data_streamer_t<depth_data_t> dataStreamer,
                    internal_token_data_t *symbol);
  ~spot_order_book_t();
  void run() override;

private:
#ifdef _DEBUG
  void printOrderBook();
#endif

  void match(backtesting::order_data_t order) override;
  void cancel(backtesting::order_data_t order) override;

  void setNextTimer();
  void shakeOrderBook();
  void updateOrderBook(depth_data_t &&newestData);
  [[nodiscard]] trade_data_t getNewTrade(order_data_t const &order,
                                         order_status_e const, double const qty,
                                         double const amount);
  [[nodiscard]] trade_list_t
  getExecutedTradesFromOrders(details::spot_order_meta_data_t &data,
                              double quantityTraded, double const priceLevel);
  [[nodiscard]] trade_list_t
  marketMatcher(std::vector<details::spot_order_meta_data_t> &list,
                double &amountAvailableToSpend, order_data_t const &order);

private:
  details::spot_order_book_meta_t m_orderBook;
  depth_data_t m_nextData;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  time_t m_currentTimer;
};

details::spot_order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       internal_token_data_t *token, trade_side_e const side,
                       trade_type_e const tradeType);
} // namespace backtesting
