#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <optional>

#include "common.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websock = beast::websocket;
namespace http = beast::http;
namespace ip = net::ip;

namespace binance {
class websocket_stream_base_t {
  using resolver_result_type = net::ip::tcp::resolver::results_type;
  using resolver = ip::tcp::resolver;
  using results_type = resolver::results_type;

public:
  websocket_stream_base_t(net::io_context &ioContext,
                          net::ssl::context &sslContext,
                          trade_type_e const tradeType)
      : m_ioContext(ioContext), m_sslContext(sslContext),
        m_hostName(tradeType == trade_type_e::futures ? url_t::futuresHostName
                                                      : url_t::spotHostName),
        m_portNumber(tradeType == trade_type_e::spot
                         ? url_t::spotPortNumber
                         : url_t::futuresPortNumber) {}
  virtual ~websocket_stream_base_t();
  void start();
  void makeSubscriptionsFor(std::vector<std::string> const &tokens) {
    for (auto const &token : tokens)
      insertIfNotExists(token);
  }

  void makeSubscriptionFor(std::string const &token) {
    insertIfNotExists(token);
  }

protected:
  virtual std::string
  getSubscriptionMessage(std::string const &tokenName) const = 0;
  virtual std::string getStreamType() const = 0;
  virtual void processResponse(char const *const str, size_t const length) = 0;

protected:
private:
  void websockConnectToResolvedNames(results_type const &results);
  void websockPerformSslHandshake(results_type::endpoint_type const &name);
  void negotiateWebsocketConnection();
  void performWebsocketHandshake();
  void waitForMessages();
  void interpretGenericMessages();

  inline void restart() {
    for (auto &d : m_tokens)
      d.subscribedFor = false;
    m_allTokensSubscribedFor = false;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    start();
  }

  void insertIfNotExists(std::string const &token);
  void makeSubscription(internal_token_data_t *);
  internal_token_data_t *getNonSubcribedForToken();

private:
  net::io_context &m_ioContext;
  net::ssl::context &m_sslContext;
  std::optional<resolver> m_resolver = std::nullopt;
  std::optional<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>
      m_sslWebStream;
  std::optional<beast::flat_buffer> m_readBuffer;
  std::vector<internal_token_data_t> m_tokens;
  std::string m_writeBuffer;
  char const *const m_hostName;
  char const *const m_portNumber;
  bool m_allTokensSubscribedFor = false;
};

std::string toLowerString(std::string const &s);
} // namespace binance
