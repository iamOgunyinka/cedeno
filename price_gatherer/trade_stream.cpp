#include "trade_stream.hpp"
#include <iostream>

namespace binance {
std::string
trade_stream_t::getSubscriptionMessage(std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@aggTrade\"],\"id\": 30}";
}

void trade_stream_t::processResponse(char const *const str,
                                     size_t const length) const {
  (void)length;
  std::cout << "AggTrade:" << str << std::endl;
}
} // namespace binance
