#ifndef SOURCE_DATA_HPP_
#define SOURCE_DATA_HPP_
#include <string>

namespace indicators{

enum class side_e{
  none,
  sell,
  buy,
};

struct kline_t{
    std::string tokenName = "symbol";
    double closePrice = 0.0;
    double highPrice = 0.0;
    double lowPrice = 0.0;
};

struct trade_stream_t{
  std::string tokenName;
  uint64_t eventTime = 0;
  double quantityExecuted = 0.0;
  double amountPerPiece = 0.0;
  side_e side = side_e::none;
};

using trade_stream_d = trade_stream_t;
using kline_d = kline_t;

}


#endif