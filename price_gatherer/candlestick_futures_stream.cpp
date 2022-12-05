#include "candlestick_futures_stream.hpp"

#include <iostream>
#include <rapidjson/document.h>

namespace binance {
char const *const candlestick_futures_stream_t::api_host = "fapi.binance.com";
char const *const candlestick_futures_stream_t::ws_host = "fstream.binance.com";
char const *const candlestick_futures_stream_t::ws_port_number = "443";

void candlestick_futures_stream_t::onResultAvailable(candlestick_data_t data) {
  std::cout << "FUTURES: " << data.tokenName << " -> " << data.lowPrice
            << " -> " << data.highPrice << std::endl;
}

} // namespace binance
