#include "ticker_stream.hpp"
#include <iostream>

namespace binance {
std::string
ticker_stream_t::getSubscriptionMessage(std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@ticker\"],\"id\": 40}";
}

void ticker_stream_t::processResponse(char const *const str,
                                      size_t const length) const {
  (void)length;
  std::cout << "Ticker:" << str << std::endl;
}

void ticker_stream_t::writeCSVHeader() {}
// ========================================================

std::string book_ticker_stream_t::getSubscriptionMessage(
    std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@bookTicker\"],\"id\": 50}";
}

void book_ticker_stream_t::processResponse(char const *const str,
                                           size_t const length) const {
  (void)length;
  std::cout << "BookerTicker:" << str << std::endl;
}

/*
// SPOT
* {
  "u":400900217,     // order book updateId
  "s":"BNBUSDT",     // symbol
  "b":"25.35190000", // best bid price
  "B":"31.21000000", // best bid qty
  "a":"25.36520000", // best ask price
  "A":"40.66000000"  // best ask qty
}

// FUTURES
{
  "e":"bookTicker",         // event type
  "u":400900217,            // order book updateId
  "E": 1568014460893,       // event time
  "T": 1568014460891,       // transaction time
  "s":"BNBUSDT",            // symbol
  "b":"25.35190000",        // best bid price
  "B":"31.21000000",        // best bid qty
  "a":"25.36520000",        // best ask price
  "A":"40.66000000"         // best ask qty
}

*/
void book_ticker_stream_t::writeCSVHeader() {
  for (auto &[_, file] : m_tradeMap.dataMap) {
    if (file.isOpen()) {
      file << "" << std::endl;
    }
  }
}
} // namespace binance
