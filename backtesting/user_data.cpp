#include "user_data.hpp"
#include "global_data.hpp"
#include <algorithm>

namespace backtesting {
namespace utils {
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
}

std::optional<order_data_t>
user_data_t::createOrder(std::string const &tokenName, double const quantity,
                         double const price, double const leverage,
                         trade_type_e const type, trade_side_e const side,
                         trade_market_e const market) {
  auto &globalRtData = global_data_t::instance();
  auto &tokenList = globalRtData.allTokens;
  auto findIter = std::find_if(tokenList.cbegin(), tokenList.cend(),
                               [tokenName, type](token_data_t const &data) {
                                 return utils::isCaseInsensitiveStringCompare(
                                            data.name, tokenName) &&
                                        (type == data.tradeType);
                               });
  if (findIter == tokenList.cend())
    return std::nullopt;
  /*
  auto iter =
      std::find_if(assets.cbegin(), assets.cend(),
                   [tokenName = findIter->name](user_asset_t const &asset) {
                     return utils::isCaseInsensitiveStringCompare(
                         asset.tokenName, tokenName);
                   });
  if (iter == assets.cend()) {
    //
  }
  */

  return createOrderImpl(findIter->name, quantity, price, leverage,
                         findIter->tradeType, side, market);
}

std::optional<order_data_t>
user_data_t::createOrder(uint64_t const tokenID, double const quantity,
                         double const price, double const leverage,
                         trade_side_e const side, trade_market_e const market) {
  auto &globalRtData = global_data_t::instance();
  auto &tokens = globalRtData.allTokens;
  auto iter = std::find_if(
      tokens.cbegin(), tokens.cend(),
      [tokenID](token_data_t const &t) { return t.tokenID == tokenID; });
  if (iter == tokens.cend())
    return std::nullopt;
  return createOrderImpl(iter->name, quantity, price, leverage, iter->tradeType,
                         side, market);
}

order_data_t user_data_t::createOrderImpl(
    std::string const &tokenName, double const quantity, double const price,
    double const leverage, trade_type_e const tradeType,
    trade_side_e const side, trade_market_e const market) {
  order_data_t order;
  order.leverage = leverage;
  order.market = market;
  order.priceLevel = price;
  order.quantity = quantity;
  order.side = side;
  order.tokenName = tokenName;
  order.type = tradeType;
  order.user = this;
  return order;
}

} // namespace backtesting
