#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <iostream>

#include "common.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace websock = beast::websocket;
namespace http = beast::http;
namespace ip = net::ip;

namespace binance {

template <typename Derived> class candlestick_base_t {
  using resolver_result_type = net::ip::tcp::resolver::results_type;

public:
  using resolver = ip::tcp::resolver;
  using results_type = resolver::results_type;

  candlestick_base_t(net::io_context &ioContext, net::ssl::context &sslContext);
  virtual ~candlestick_base_t();
  void start();
  inline void makeSubscriptionsFor(std::vector<std::string> const &tokens) {
    for (auto const &token : tokens)
      insertIfNotExists(token);
  }

  inline void makeSubscriptionFor(std::string const &token) {
    insertIfNotExists(token);
  }

private:
  inline auto shared_from_this() { return this; }
  inline void restart() {
    resetTokensToDefault();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    start();
  }
  inline void resetTokensToDefault() {
    m_allTokensSubscribedFor = false;
    for (auto &tokenInfo : m_tokens)
      tokenInfo.subscribedFor = false;
  }

  void websockConnectToResolvedNames(results_type const &results);
  void websockPerformSslHandshake(results_type::endpoint_type const &name);
  void negotiateWebsocketConnection();
  void performWebsocketHandshake();
  void waitForMessages();
  void interpretGenericMessages();
  void insertIfNotExists(std::string const &token);
  void makeSubscription(internal_token_data_t *);
  internal_token_data_t *getNonSubcribedForToken();

protected:
  net::io_context &m_ioContext;
  net::ssl::context &m_sslContext;
  std::optional<resolver> m_resolver = std::nullopt;
  std::optional<beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>
      m_sslWebStream;
  std::optional<beast::flat_buffer> m_readBuffer;
  std::vector<internal_token_data_t> m_tokens;
  std::string m_writeBuffer;
  bool m_allTokensSubscribedFor = false;
};

template <typename T>
candlestick_base_t<T>::candlestick_base_t(net::io_context &ioContext,
                                          net::ssl::context &sslContext)
    : m_ioContext(ioContext), m_sslContext(sslContext) {}

template <typename T> candlestick_base_t<T>::~candlestick_base_t() {
  m_tokens.clear();
  m_resolver.reset();
  m_sslWebStream.reset();
  m_readBuffer.reset();
}

template <typename T>
void candlestick_base_t<T>::insertIfNotExists(std::string const &tokenName) {
  auto const t = toLowerString(tokenName);
  auto const iter =
      std::find_if(m_tokens.cbegin(), m_tokens.cend(),
                   [&t](internal_token_data_t const &internalToken) {
                     return internalToken.tokenName == t;
                   });
  if (iter == m_tokens.cend())
    m_tokens.push_back({t, false});
}

template <typename T> void candlestick_base_t<T>::start() {
  if (m_tokens.empty()) {
    std::cerr << __func__ << ": no tokens exists to be subscribed for."
              << std::endl;
    return;
  }

  auto const host = static_cast<T *>(this)->getWsHost();
  auto const portNumber = static_cast<T *>(this)->getWsPortNumber();

  m_resolver.emplace(m_ioContext);
  m_resolver->async_resolve(
      host, portNumber,
      [self = shared_from_this()](auto const error_code,
                                  results_type const &results) {
        if (error_code || results.empty()) {
          std::cerr << error_code.message() << std::endl;
          return self->restart();
        }
        self->websockConnectToResolvedNames(results);
      });
}

template <typename T>
void candlestick_base_t<T>::websockConnectToResolvedNames(
    results_type const &resolvedNames) {
  m_resolver.reset();
  m_sslWebStream.emplace(m_ioContext, m_sslContext);
  beast::get_lowest_layer(*m_sslWebStream)
      .expires_after(std::chrono::seconds(30));
  beast::get_lowest_layer(*m_sslWebStream)
      .async_connect(
          resolvedNames,
          [self = shared_from_this()](
              auto const &errorCode,
              resolver_result_type::endpoint_type const &connectedName) {
            if (errorCode) {
              std::cerr << errorCode.message() << std::endl;
              return self->restart();
            }
            self->websockPerformSslHandshake(connectedName);
          });
}

template <typename T>
void candlestick_base_t<T>::websockPerformSslHandshake(
    results_type::endpoint_type const &name) {
  std::string const baseHost = static_cast<T *>(this)->getWsHost();
  auto const fullHost = baseHost + ":" + std::to_string(name.port());

  // Set a timeout on the operation
  beast::get_lowest_layer(*m_sslWebStream)
      .expires_after(std::chrono::seconds(30));

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(m_sslWebStream->next_layer().native_handle(),
                                fullHost.c_str())) {
    auto const ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                                      net::error::get_ssl_category());
    std::cerr << ec.message() << std::endl;
    return;
  }
  negotiateWebsocketConnection();
}

template <typename T>
void candlestick_base_t<T>::negotiateWebsocketConnection() {
  m_sslWebStream->next_layer().async_handshake(
      net::ssl::stream_base::client,
      [self = shared_from_this()](beast::error_code const ec) {
        if (ec) {
          std::cerr << ec.message() << std::endl;
          return self->restart();
        }
        beast::get_lowest_layer(*self->m_sslWebStream).expires_never();
        self->performWebsocketHandshake();
      });
}

template <typename T>
internal_token_data_t *candlestick_base_t<T>::getNonSubcribedForToken() {
  for (size_t i = 0; i < m_tokens.size(); ++i)
    if (m_tokens[i].subscribedFor == false)
      return &m_tokens[i];
  return nullptr;
}

template <typename T> void candlestick_base_t<T>::performWebsocketHandshake() {
  auto token = getNonSubcribedForToken();
  if (!token) {
    std::cerr << __func__ << ": no token to returned in getNonSubcribedForToken"
              << std::endl;
    return;
  }
  token->subscribedFor = true;
  auto opt = beast::websocket::stream_base::timeout();
  opt.idle_timeout = std::chrono::seconds(50);
  opt.handshake_timeout = std::chrono::seconds(20);
  opt.keep_alive_pings = true;
  m_sslWebStream->set_option(opt);

  m_sslWebStream->control_callback(
      [self = shared_from_this()](auto const frame_type, auto const &) {
        if (frame_type == beast::websocket::frame_type::close) {
          std::cerr << "Stream closed" << std::endl;
          return self->restart();
        }
      });

  auto const host = static_cast<T *>(this)->getWsHost();
  auto const handshakePath =
      static_cast<T *>(this)->klineHandshakePath(token->tokenName);
  m_sslWebStream->async_handshake(
      host, handshakePath,
      [self = shared_from_this()](beast::error_code const ec) {
        if (ec) {
          std::cerr << ec.message() << std::endl;
          return self->restart();
        }

        self->waitForMessages();
      });
}

template <typename T> void candlestick_base_t<T>::waitForMessages() {
  m_readBuffer.emplace();
  m_sslWebStream->async_read(
      *m_readBuffer,
      [self = shared_from_this()](beast::error_code const error_code,
                                  std::size_t const) {
        if (error_code == net::error::operation_aborted) {
          std::cerr << error_code.message() << std::endl;
          return;
        } else if (error_code) {
          std::cerr << error_code.message() << std::endl;
          return self->restart();
        }
        self->interpretGenericMessages();
      });
}

template <typename T> void candlestick_base_t<T>::interpretGenericMessages() {
  assert(m_readBuffer.has_value());

  char const *bufferCstr =
      static_cast<char const *>(m_readBuffer->cdata().data());

  // do something with the message
  if (auto data = parseCandleStickData(bufferCstr, m_readBuffer->size()); data)
    static_cast<T *>(this)->onResultAvailable(*data);

  if (!m_allTokensSubscribedFor) {
    if (auto t = getNonSubcribedForToken(); t != nullptr)
      return makeSubscription(t);
    m_allTokensSubscribedFor = true;
  }

  waitForMessages();
}

template <typename T>
void candlestick_base_t<T>::makeSubscription(internal_token_data_t *token) {
  m_writeBuffer = static_cast<T *>(this)->subscriptionMessage(token->tokenName);

  m_sslWebStream->async_write(
      net::buffer(m_writeBuffer),
      [self = shared_from_this(), token](auto const errCode, size_t const) {
        if (errCode) {
          std::cerr << errCode.message() << std::endl;
          return self->restart();
        }
        self->m_writeBuffer.clear();
        token->subscribedFor = true;
        self->waitForMessages();
      });
}

} // namespace binance
