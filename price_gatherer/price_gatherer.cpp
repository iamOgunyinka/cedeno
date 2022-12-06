#include "trade_stream.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <iostream>

// #include <thread>

namespace net = boost::asio;

int main(int const argc, char const **argv) {
  // auto const maxThreadSize = std::thread::hardware_concurrency();
  net::io_context ioContext;
  auto sslContext =
      std::make_unique<net::ssl::context>(net::ssl::context::tlsv12_client);
  sslContext->set_default_verify_paths();
  sslContext->set_verify_mode(boost::asio::ssl::verify_none);

  std::vector<std::string> const tokens{"BNBUSDT", "BTCUSDT", "RUNEUSDT",
                                        "ETHUSDT"};
  binance::trade_stream_t tradeStream(ioContext, *sslContext,
                                      binance::trade_type_e::spot);
  tradeStream.makeSubscriptionsFor(tokens);
  tradeStream.start();

  ioContext.run();
  return 0;
}
