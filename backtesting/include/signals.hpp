#pragma once

#include <Signals/Delegate.h>
#include <unordered_map>

namespace backtesting {
struct internal_token_data_t;

struct signals_t {
  using price_delegate_t = Gallant::Delegate2<internal_token_data_t *, double>;
  static std::unordered_map<internal_token_data_t *, double> latestPrices;
  static void OnNewFuturesPrice(internal_token_data_t *, double const);
  static double currentPrice(internal_token_data_t *);
  static price_delegate_t &GetPriceDelegate();
};

void liquidationOfPositionsImpl();
} // namespace backtesting
