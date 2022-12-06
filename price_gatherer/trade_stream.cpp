#include "trade_stream.hpp"
#include <iostream>

namespace binance {
char const *const trade_stream_t::spotHostName = "stream.binance.com";
char const *const trade_stream_t::futuresHostName = "fstream.binance.com";
char const *const trade_stream_t::spotPortNumber = "9443";
char const *const trade_stream_t::futuresPortNumber = "443";

trade_stream_t::~trade_stream_t() {
  m_resolver.reset();
  m_sslWebStream.reset();
  m_readBuffer.reset();
}

void trade_stream_t::start() {

  m_resolver.emplace(m_ioContext);
  m_resolver->async_resolve(
      m_hostName, m_portNumber,
      [this](auto const error_code, results_type const &results) {
        if (error_code || results.empty()) {
          std::cerr << error_code.message() << std::endl;
          return;
        }
        websockConnectToResolvedNames(results);
      });
}

void trade_stream_t::websockConnectToResolvedNames(
    results_type const &resolvedNames) {
  m_resolver.reset();
  m_sslWebStream.emplace(m_ioContext, m_sslContext);
  beast::get_lowest_layer(*m_sslWebStream)
      .expires_after(std::chrono::seconds(30));
  beast::get_lowest_layer(*m_sslWebStream)
      .async_connect(
          resolvedNames,
          [this](auto const &errorCode,
                 resolver_result_type::endpoint_type const &connectedName) {
            if (errorCode) {
              std::cerr << errorCode.message() << std::endl;
              return;
            }
            websockPerformSslHandshake(connectedName);
          });
}

void trade_stream_t::websockPerformSslHandshake(
    results_type::endpoint_type const &name) {
  auto const fullHost =
      m_hostName + std::string(":") + std::to_string(name.port());

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

void trade_stream_t::insertIfNotExists(std::string const &tokenName) {
  auto const t = toLowerString(tokenName);
  auto const iter =
      std::find_if(m_tokens.cbegin(), m_tokens.cend(),
                   [&t](internal_token_data_t const &internalToken) {
                     return internalToken.tokenName == t;
                   });
  if (iter == m_tokens.cend())
    m_tokens.push_back({t, false});
}

typename trade_stream_t::internal_token_data_t *
trade_stream_t::getNonSubcribedForToken() {
  for (size_t i = 0; i < m_tokens.size(); ++i)
    if (m_tokens[i].subscribedFor == false)
      return &m_tokens[i];
  return nullptr;
}

void trade_stream_t::makeSubscription(internal_token_data_t *token) {
  m_writeBuffer = "{\"method\": \"SUBSCRIBE\", \"params\":["
                  "\"" +
                  token->tokenName + "@aggTrade\"],\"id\": 30}";
  std::cout << "WriteBuffer:" << m_writeBuffer << std::endl;

  m_sslWebStream->async_write(net::buffer(m_writeBuffer),
                              [this, token](auto const errCode, size_t const) {
                                if (errCode) {
                                  std::cerr << errCode.message() << std::endl;
                                  return;
                                }
                                m_writeBuffer.clear();
                                token->subscribedFor = true;
                                waitForMessages();
                              });
}

void trade_stream_t::negotiateWebsocketConnection() {
  m_sslWebStream->next_layer().async_handshake(
      net::ssl::stream_base::client, [this](beast::error_code const ec) {
        if (ec) {
          std::cerr << ec.message() << std::endl;
          return;
        }
        beast::get_lowest_layer(*m_sslWebStream).expires_never();
        performWebsocketHandshake();
      });
}

void trade_stream_t::performWebsocketHandshake() {
  auto token = getNonSubcribedForToken();
  if (!token) {
    std::cerr << __func__ << ": no token to return in getNonSubcribedForToken"
              << std::endl;
    return;
  }
  token->subscribedFor = true;

  auto opt = beast::websocket::stream_base::timeout();
  opt.idle_timeout = std::chrono::seconds(50);
  opt.handshake_timeout = std::chrono::seconds(20);
  opt.keep_alive_pings = true;
  m_sslWebStream->set_option(opt);

  m_sslWebStream->control_callback([this](auto const frame_type, auto const &) {
    if (frame_type == beast::websocket::frame_type::close) {
      std::cerr << "Stream closed" << std::endl;
      m_sslWebStream.reset();
      return restart();
    }
  });
  auto const handshakePath =
      "/stream?streams=" + token->tokenName + "@aggTrade";
  m_sslWebStream->async_handshake(m_hostName, handshakePath,
                                  [this](beast::error_code const ec) {
                                    if (ec) {
                                      std::cerr << ec.message() << std::endl;
                                      return;
                                    }

                                    waitForMessages();
                                  });
}

void trade_stream_t::waitForMessages() {
  m_readBuffer.emplace();
  m_sslWebStream->async_read(
      *m_readBuffer,
      [this](beast::error_code const error_code, std::size_t const) {
        if (error_code == net::error::operation_aborted) {
          std::cerr << error_code.message() << std::endl;
          return;
        } else if (error_code) {
          std::cerr << error_code.message() << std::endl;
          return restart();
        }
        interpretGenericMessages();
      });
}

void trade_stream_t::interpretGenericMessages() {
  assert(m_readBuffer.has_value());

  char const *bufferCstr =
      static_cast<char const *>(m_readBuffer->cdata().data());

  // do something with the message
  std::cout << bufferCstr << std::endl;

  if (!m_allTokensSubscribedFor) {
    if (auto t = getNonSubcribedForToken(); t != nullptr)
      return makeSubscription(t);
    m_allTokensSubscribedFor = true;
  }

  waitForMessages();
}

} // namespace binance
