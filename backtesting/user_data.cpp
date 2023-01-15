#include "user_data.hpp"
#include "global_data.hpp"
#include <algorithm>

namespace backtesting {
namespace utils {
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
}

void user_data_t::OnNewTrade(order_data_t const &order,
                             double const quantityExecuted, double const cost) {
  auto assetIter =
      std::find_if(assets.begin(), assets.end(),
                   [tokenName = order.tokenName](user_asset_t const &asset) {
                     return utils::isCaseInsensitiveStringCompare(
                         asset.tokenName, tokenName);
                   });
  if (assetIter == assets.end())
    throw std::logic_error("asset spent is not available");

  if (order.side == trade_side_e::buy) {
    assetIter->base.amountAvailable += quantityExecuted;
    assetIter->quote.amountInUse -= cost;
  } else if (order.side == trade_side_e::sell) {
    assetIter->quote.amountAvailable += cost;
    assetIter->base.amountInUse -= quantityExecuted;
  }
}

bool user_data_t::hasTradableBalance(std::string const &tokenName,
                                     trade_type_e const tradeType,
                                     trade_side_e const side,
                                     double const quantity,
                                     double const price) {
  auto assetIter = std::find_if(assets.begin(), assets.end(),
                                [tokenName](user_asset_t const &asset) {
                                  return utils::isCaseInsensitiveStringCompare(
                                      asset.tokenName, tokenName);
                                });
  auto const totalAmountNeeded = quantity * price;
  if (assetIter == assets.end())
    return false;

  if (side == trade_side_e::buy) {
    auto &token = assetIter->quote;
    if (token.amountAvailable < totalAmountNeeded)
      return false;
    token.amountAvailable -= totalAmountNeeded;
    token.amountInUse += totalAmountNeeded;
  } else if (side == trade_side_e::sell) {
    auto &token = assetIter->base;
    if (token.amountAvailable < totalAmountNeeded)
      return false;
    token.amountAvailable -= totalAmountNeeded;
    token.amountInUse += totalAmountNeeded;
  }
  return true;
}

bool user_data_t::isValidTradeToken(std::string const &tokenName,
                                    trade_type_e const tradeType) const {
  auto &globalRtData = global_data_t::instance();
  auto &tokenList = globalRtData.allTokens;
  auto findIter = std::find_if(
      tokenList.cbegin(), tokenList.cend(),
      [tokenName, tradeType](token_data_t const &data) {
        return utils::isCaseInsensitiveStringCompare(data.name, tokenName) &&
               (tradeType == data.tradeType);
      });
  return findIter != tokenList.cend();
}

std::optional<order_data_t>
user_data_t::getLimitOrder(std::string const &tokenName, double const quantity,
                           double const price, double const leverage,
                           trade_type_e const type, trade_side_e const side) {
  if (!(isValidTradeToken(tokenName, type) &&
        hasTradableBalance(tokenName, type, side, quantity, price)))
    return std::nullopt;

  return createOrderImpl(tokenName, quantity, price, leverage, type, side,
                         trade_market_e::limit);
}

std::optional<order_data_t>
user_data_t::getMarketOrder(std::string const &tokenName, double const price,
                            double const leverage, trade_type_e const tradeType,
                            trade_side_e const side) {
  if (!(isValidTradeToken(tokenName, tradeType) &&
        hasTradableBalance(tokenName, tradeType, side, 1.0, price)))
    return std::nullopt;

  return createOrderImpl(tokenName, 1.0, price, leverage, tradeType, side,
                         trade_market_e::market);
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
