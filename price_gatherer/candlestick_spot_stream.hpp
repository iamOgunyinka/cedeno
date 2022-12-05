#pragma once

#include "candlestick_base.hpp"

namespace binance {

class candlestick_spot_stream_t
    : public candlestick_base_t<candlestick_spot_stream_t> {
  static char const *const api_host;
  static char const *const ws_host;
  static char const *const ws_port_number;

public:
  candlestick_spot_stream_t(net::io_context &ioContext,
                            net::ssl::context &sslContext)
      : candlestick_base_t(ioContext, sslContext) {}
  ~candlestick_spot_stream_t() {}
  void start() { candlestick_base_t::start(); }
  void onResultAvailable(candlestick_data_t);

  inline std::string getWsHost() const { return ws_host; }
  inline std::string getWsPortNumber() const { return ws_port_number; }
  inline std::string getHttpHost() const { return api_host; }
  inline std::string klineHandshakePath(std::string const &tokenName) const {
    return "/stream?streams=" + tokenName + "@kline_1s";
  }
  inline std::string subscriptionMessage(std::string const &tokenName) const {
    return "{\"method\": \"SUBSCRIBE\", \"params\":["
           "\"" +
           tokenName + "@kline_1s\"],\"id\": 10}";
  }

};
} // namespace binance
