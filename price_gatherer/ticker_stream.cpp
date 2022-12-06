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

} // namespace binance
