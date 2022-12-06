#include "candlestick_spot_stream.hpp"
#include <iostream>

namespace binance {

void candlestick_spot_stream_t::onResultAvailable(candlestick_data_t data) {
  std::cout << "SPOT: " << data.tokenName << " -> " << data.lowPrice << " -> "
            << data.highPrice << std::endl;
}

} // namespace binance
