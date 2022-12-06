#pragma once

#include "websocket_stream_base.hpp"

namespace binance {
class trade_stream_t : public websocket_stream_base_t {

public:
  trade_stream_t(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_type_e const tradeType)
      : websocket_stream_base_t(ioContext, sslContext, tradeType) {}
  ~trade_stream_t() {}

private:
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "aggTrade"; }
  void processResponse(char const *const str,
                       size_t const length) const override;
};
} // namespace binance
