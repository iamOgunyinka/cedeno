#pragma once

#include "candlestick_base.hpp"
#include "common.hpp"

namespace binance {

class candlestick_spot_stream_t
    : public candlestick_base_t<candlestick_spot_stream_t> {
public:
  candlestick_spot_stream_t(net::io_context &ioContext,
                            net::ssl::context &sslContext,
                            token_filename_map_t &tradeMap)
      : candlestick_base_t(ioContext, sslContext), m_tradeMap(tradeMap) {
    for (auto const &[key, _] : tradeMap.dataMap)
      makeSubscriptionFor(key);
  }
  ~candlestick_spot_stream_t() {
    for (auto &[_, file] : m_tradeMap.dataMap) {
      file.close();
    }
  }
  void start() { candlestick_base_t::start(); }
  void onResultAvailable(candlestick_data_t const &);

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

private:
  void writeCSVHeader(binance::locked_file_t&);

  token_filename_map_t &m_tradeMap;
};
} // namespace binance
