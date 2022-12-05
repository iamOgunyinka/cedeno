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
  void onResultAvailable(candlestick_data_t);

  inline std::string getWsHost() const { return ws_host; }
  inline std::string getWsPortNumber() const { return ws_port_number; }
  inline std::string getHttpHost() const { return api_host; }
  inline std::string klineHandshakePath(std::string const &tokenName) const {
    return "/stream?streams=" + tokenName + "@kline_1m";
  }

  inline std::string subscriptionMessage(std::string const &tokenName) const {
    return "{\"method\": \"SUBSCRIBE\", \"params\":["
           "\"" +
           tokenName + "@kline_1m\"],\"id\": 20}";
  }
};

} // namespace binance
