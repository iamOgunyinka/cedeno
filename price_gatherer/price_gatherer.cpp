#include "candlestick_spot_stream.hpp"
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

  binance::candlestick_spot_stream_t cs(ioContext, *sslContext);
  cs.makeSubscriptionsFor({"BTCUSDT", "RUNEUSDT", "ETHUSDT"});
  cs.makeSubscriptionFor("BNBUSDT");
  cs.start();

  ioContext.run();
  return 0;
}
