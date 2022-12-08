#pragma once

#include "websocket_stream_base.hpp"

namespace binance {
class ticker_stream_t : public websocket_stream_base_t {

public:
  ticker_stream_t(net::io_context &ioContext, net::ssl::context &sslContext,
                  trade_type_e const tradeType, token_filename_map_t &tradeMap)
      : websocket_stream_base_t(ioContext, sslContext, tradeType),
        m_tradeMap(tradeMap), m_tradeType(tradeType) {
    for (auto const &[key, _] : tradeMap.dataMap)
      makeSubscriptionFor(key);
    writeCSVHeader();
  }
  ~ticker_stream_t() {}

private:
  void writeCSVHeader();
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "ticker"; }
  void processResponse(char const *const str, size_t const length) override;

  token_filename_map_t &m_tradeMap;
  trade_type_e const m_tradeType;
  int m_flushInterval = 0;
};

class book_ticker_stream_t : public websocket_stream_base_t {

public:
  book_ticker_stream_t(net::io_context &ioContext,
                       net::ssl::context &sslContext,
                       trade_type_e const tradeType,
                       token_filename_map_t &tradeMap)
      : websocket_stream_base_t(ioContext, sslContext, tradeType),
        m_tradeMap(tradeMap) {
    for (auto const &[key, _] : tradeMap.dataMap)
      makeSubscriptionFor(key);
    writeCSVHeader();
  }
  ~book_ticker_stream_t() {}

private:
  void writeCSVHeader();
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "bookTicker"; }
  void processResponse(char const *const str, size_t const length) override;

  int m_flushInterval = 0;
  token_filename_map_t &m_tradeMap;
};

} // namespace binance
