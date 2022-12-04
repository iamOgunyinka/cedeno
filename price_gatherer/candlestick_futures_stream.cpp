#include "candlestick_futures_stream.hpp"

#include <iostream>
#include <rapidjson/document.h>

namespace binance {
char const *const candlestick_futures_stream_t::api_host = "fapi.binance.com";
char const *const candlestick_futures_stream_t::ws_host = "fstream.binance.com";
char const *const candlestick_futures_stream_t::ws_port_number = "443";

std::optional<candlestick_futures_data_t>
parseFuturesCandleStickData(char const *const str, size_t const length) {
#ifdef _MSC_VER
#undef GetObject
#endif

  rapidjson::Document d;
  d.Parse(str, length);

  if (!d.IsObject())
    return std::nullopt;

  auto const jsonObject = d.GetObject();
  return std::nullopt;
}

void candlestick_futures_stream_t::processResult(char const *const str,
                                                 size_t const size) {
  [[maybe_unused]] auto const t = parseFuturesCandleStickData(str, size);
  if (!t)
    return;
}

} // namespace binance
