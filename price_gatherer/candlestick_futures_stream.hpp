#pragma once

#include "candlestick_base.hpp"
#include "common.hpp"

namespace binance {

class candlestick_futures_stream_t
    : public candlestick_base_t<candlestick_futures_stream_t> {

public:
  candlestick_futures_stream_t(net::io_context &ioContext,
                               net::ssl::context &sslContext,
                               token_filename_map_t &tradeMap)
      : candlestick_base_t(ioContext, sslContext), m_tradeMap(tradeMap) {
    for (auto const &[key, _] : tradeMap.dataMap)
      makeSubscriptionFor(key);
  }
  ~candlestick_futures_stream_t() {}
  void start() { candlestick_base_t::start(); }
  void onResultAvailable(candlestick_data_t);

  inline std::string getWsHost() const { return url_t::futuresHostName; }
  inline std::string getWsPortNumber() const {
    return url_t::futuresPortNumber;
  }
  inline std::string klineHandshakePath(std::string const &tokenName) const {
    return "/stream?streams=" + tokenName + "@kline_1m";
  }

  inline std::string subscriptionMessage(std::string const &tokenName) const {
    return "{\"method\": \"SUBSCRIBE\", \"params\":["
           "\"" +
           tokenName + "@kline_1m\"],\"id\": 20}";
  }

private:
  token_filename_map_t &m_tradeMap;
};

} // namespace binance
