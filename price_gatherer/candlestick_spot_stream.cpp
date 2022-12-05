#include "candlestick_spot_stream.hpp"

#include <iostream>

namespace binance {

char const *const candlestick_spot_stream_t::api_host = "api.binance.com";
char const *const candlestick_spot_stream_t::ws_host = "stream.binance.com";
char const *const candlestick_spot_stream_t::ws_port_number = "9443";

void candlestick_spot_stream_t::onResultAvailable(candlestick_data_t data) {
  std::cout << "SPOTS: " << data.tokenName << " -> " << data.lowPrice << " -> "
            << data.highPrice << std::endl;
}

} // namespace binance
