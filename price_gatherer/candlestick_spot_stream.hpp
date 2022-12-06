#pragma once

#include "candlestick_base.hpp"
#include "common.hpp"

namespace binance {

class candlestick_spot_stream_t
    : public candlestick_base_t<candlestick_spot_stream_t> {
public:
  candlestick_spot_stream_t(net::io_context &ioContext,
                            net::ssl::context &sslContext)
      : candlestick_base_t(ioContext, sslContext) {}
  ~candlestick_spot_stream_t() {}
  void start() { candlestick_base_t::start(); }
  void onResultAvailable(candlestick_data_t);

  inline std::string getWsHost() const { return url_t::spotHostName; }
  inline std::string getWsPortNumber() const { return url_t::spotPortNumber; }
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
