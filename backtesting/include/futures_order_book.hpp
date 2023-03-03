#pragma once

#include "futures_data.hpp"
#include "order_book_base.hpp"

namespace backtesting {

namespace futures {
struct order_meta_data_t {
  double quantity = 0.0;
  double priceLevel = 0.0;
  std::vector<order_data_t> orders;
};
} // namespace futures

class futures_order_book_t : public order_book_base_t {
public:
  futures_order_book_t(net::io_context &ioContext,
                       data_streamer_t<depth_data_t> dataStream,
                       internal_token_data_t *token, double const makerFee,
                       double const takerFee);
  ~futures_order_book_t() {}

private:
#ifdef _DEBUG
  void printOrderBook() override {}
#endif

  void match(backtesting::order_data_t order) override;
  void cancel(backtesting::order_data_t order) override;
  void shakeOrderBook() override;

  double const m_takerFee;
  double const m_makerFee;
};
} // namespace backtesting
