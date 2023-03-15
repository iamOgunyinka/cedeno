#ifndef SOURCE_DATA_HPP_
#define SOURCE_DATA_HPP_
#include <string>

#include "trades_data.hpp"
#include "candlestick_data.hpp"
namespace indicators{

#define USE_BACKTESTING_SOURCE_DATA 0

enum class side_test_e{
  none,
  sell,
  buy,
};

struct kline_test_t{
    std::string tokenName = "symbol";
    double closePrice = 0.0;
    double highPrice = 0.0;
    double lowPrice = 0.0;
};

struct trade_stream_test_t{
  std::string tokenName;
  uint64_t eventTime = 0;
  double quantityExecuted = 0.0;
  double amountPerPiece = 0.0;
  side_test_e side = side_test_e::none;
};

#if (USE_BACKTESTING_SOURCE_DATA)
using assest_side_e = backtesting::trade_side_e;
using trade_stream_d = backtesting::trade_data_t;
using kline_d = backtesting::binance_candlestick_data_t;
#else
using assest_side_e = side_test_e;
using trade_stream_d = trade_stream_test_t;
using kline_d = kline_test_t;
#endif

}


#endif