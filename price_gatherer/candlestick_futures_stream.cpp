#include "candlestick_futures_stream.hpp"
#include <iostream>

namespace binance {

void candlestick_futures_stream_t::onResultAvailable(candlestick_data_t data) {
  std::cout << "FUTURES: " << data.tokenName << " -> " << data.lowPrice
            << " -> " << data.highPrice << std::endl;
}

} // namespace binance
