#pragma once

#include "websocket_stream_base.hpp"

namespace binance {
class ticker_stream_t : public websocket_stream_base_t {

public:
  ticker_stream_t(net::io_context &ioContext, net::ssl::context &sslContext,
                  trade_type_e const tradeType)
      : websocket_stream_base_t(ioContext, sslContext, tradeType) {}
  ~ticker_stream_t() {}

private:
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "ticker"; }
  void processResponse(char const *const str,
                       size_t const length) const override;
};

class book_ticker_stream_t : public websocket_stream_base_t {

public:
  book_ticker_stream_t(net::io_context &ioContext,
                       net::ssl::context &sslContext,
                       trade_type_e const tradeType)
      : websocket_stream_base_t(ioContext, sslContext, tradeType) {}
  ~book_ticker_stream_t() {}

private:
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "bookTicker"; }
  void processResponse(char const *const str,
                       size_t const length) const override;
};

} // namespace binance
