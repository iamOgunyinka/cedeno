#pragma once

#include "websocket_stream_base.hpp"

namespace binance {
class trade_stream_t : public websocket_stream_base_t {

public:
  trade_stream_t(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_type_e const tradeType, token_filename_map_t &tradeMap)
      : websocket_stream_base_t(ioContext, sslContext, tradeType),
        m_tradeMap(tradeMap) {
    for (auto const &[key, _] : m_tradeMap.dataMap)
      makeSubscriptionFor(key);
  }
  ~trade_stream_t() {}

private:
  void writeCSVHeader(binance::locked_file_t&);
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "aggTrade"; }
  void processResponse(char const *const str, size_t const length) override;

private:
  token_filename_map_t &m_tradeMap;
};
} // namespace binance
