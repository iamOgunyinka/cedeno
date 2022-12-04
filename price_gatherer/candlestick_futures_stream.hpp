#pragma once

#include "candlestick_base.hpp"

namespace binance {
struct candlestick_futures_data_t {
  std::string tokenName;
};

class candlestick_futures_stream_t
    : public candlestick_base_t<candlestick_futures_stream_t> {
  static char const *const api_host;
  static char const *const ws_host;
  static char const *const ws_port_number;

public:
  candlestick_futures_stream_t(net::io_context &ioContext,
                               net::ssl::context &sslContext)
      : candlestick_base_t(ioContext, sslContext) {}
  ~candlestick_futures_stream_t() {}
  void start() { candlestick_base_t::start(); }
  std::string getWsHost() const { return ws_host; }
  std::string getWsPortNumber() const { return ws_port_number; }
  std::string getHttpHost() const { return api_host; }
  void processResult(char const *const buffer, size_t const size);
};

} // namespace binance
