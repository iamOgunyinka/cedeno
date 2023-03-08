#pragma once

#include <Signals/Delegate.h>
#include <string>
#include <unordered_map>

namespace backtesting {
struct internal_token_data_t;
enum class trade_type_e;

struct signals_t {
  using price_delegate_t = Gallant::Delegate2<internal_token_data_t *, double>;
  static std::unordered_map<internal_token_data_t *, double> latestPrices;
  static void OnNewFuturesPrice(internal_token_data_t *, double const);
  static price_delegate_t &GetPriceDelegate();
};

double currentPrice(internal_token_data_t *const);
double currentPrice(std::string const &, trade_type_e const);
void liquidationOfPositionsImpl();
} // namespace backtesting
