#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <optional>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websock = beast::websocket;
namespace http = beast::http;
namespace ip = net::ip;

namespace binance {
enum class trade_type_e {
  spot,
  futures,
};

class trade_stream_t {
  static char const *const spotHostName;
  static char const *const futuresHostName;
  static char const *const spotPortNumber;
  static char const *const futuresPortNumber;

  struct internal_token_data_t {
    std::string tokenName;
    bool subscribedFor = false;
  };

  using resolver_result_type = net::ip::tcp::resolver::results_type;
  using resolver = ip::tcp::resolver;
  using results_type = resolver::results_type;

public:
  trade_stream_t(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_type_e const tradeType)
      : m_ioContext(ioContext), m_sslContext(sslContext),
        m_hostName(tradeType == trade_type_e::futures ? futuresHostName
                                                      : spotHostName),
        m_portNumber(tradeType == trade_type_e::spot ? spotPortNumber
                                                     : futuresPortNumber) {}
  ~trade_stream_t();
  void start();
  inline void makeSubscriptionsFor(std::vector<std::string> const &tokens) {
    for (auto const &token : tokens)
      insertIfNotExists(token);
  }

  inline void makeSubscriptionFor(std::string const &token) {
    insertIfNotExists(token);
  }

private:
  void websockConnectToResolvedNames(results_type const &results);
  void websockPerformSslHandshake(results_type::endpoint_type const &name);
  void negotiateWebsocketConnection();
  void performWebsocketHandshake();
  void waitForMessages();
  void interpretGenericMessages();
  inline void restart() { start(); }

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
