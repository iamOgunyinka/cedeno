#pragma once

#include "symbol.hpp"

#ifdef BT_USE_WITH_INDICATORS
#include "manager/indc_mnger.hpp"
#include <functional>

namespace backtesting {

struct indicator_metadata_t {
  indicators::indicators_c indicator;
  internal_token_data_t *symbol = nullptr;

  explicit indicator_metadata_t(internal_token_data_t *symbol_)
      : indicator(symbol_->name), symbol(symbol_) {}
};

struct indicator_data_t {
  size_t time = 0;
  std::string symbol;
  trade_type_e tradeType;
  indicators::inf_t indicator;
};
using indicator_result_t = std::vector<indicator_data_t>;
using indicator_callback_t = std::function<void(indicator_result_t)>;
} // namespace backtesting
#endif
