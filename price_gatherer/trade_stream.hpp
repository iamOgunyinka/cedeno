#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <iostream>
#include <optional>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websock = beast::websocket;
namespace http = beast::http;
namespace ip = net::ip;

namespace binance {

class trade_stream_t {
  using resolver_result_type = net::ip::tcp::resolver::results_type;
  using resolver = ip::tcp::resolver;
  using results_type = resolver::results_type;

public:
  trade_stream_t(net::io_context &ioContext, net::ssl::context &sslContext)
      : m_ioContext(ioContext), m_sslContext(sslContext) {}
  ~trade_stream_t();
  void start();

private:
  void websockConnectToResolvedNames(results_type const &results);
  void websockPerformSslHandshake(results_type::endpoint_type const &name);
  void negotiateWebsocketConnection();
  void performWebsocketHandshake();
  void waitForMessages();
  void interpretGenericMessages();
  inline void restart() { start(); }

private:
  net::io_context &m_ioContext;
  net::ssl::context &m_sslContext;
  std::optional<resolver> m_resolver = std::nullopt;
  std::optional<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>
      m_sslWebStream;
  std::optional<beast::flat_buffer> m_readBuffer;
};

} // namespace binance
