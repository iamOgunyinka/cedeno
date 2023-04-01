#pragma once
#define BT_MILLI 20

#include "symbol.hpp"

#ifdef BT_USE_WITH_INDICATORS
#include "manager/indc_mnger.hpp"
#include <functional>

namespace backtesting {
struct indicator_metadata_t {
  std::string symbol;
  indicators::indicators_c indicator;
  explicit indicator_metadata_t(std::string const &symbol_)
      : symbol(symbol_), indicator(symbol_) {}
};

struct timeframe_info_t {
  using symbol_t = std::string;
  using indicator_list = std::vector<indicators::inf_t>;
  using data_map_t = std::map<symbol_t, indicator_list>;

  bool isClosed = false;
  data_map_t dataMap;

  indicator_list &operator[](symbol_t const &str) { return dataMap[str]; }
};

using indicator_data_t = std::map<std::string, timeframe_info_t>;
using indicator_callback_t = std::function<void(indicator_data_t)>;
} // namespace backtesting
#endif
