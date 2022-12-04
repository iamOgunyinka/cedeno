#include "candlestick_base.hpp"

namespace binance {
std::string toLowerString(std::string const &s) {
  std::string temp;
  temp.reserve(s.size());
  for (auto c : s)
    temp.push_back(std::tolower(c));
  return temp;
}


} // namespace binance
