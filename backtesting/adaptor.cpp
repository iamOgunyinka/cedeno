#include "adaptor.hpp"
#include <algorithm>

namespace backtesting {
namespace adaptor {
token_data_list_t dbTokenListToBtTokenList(db_token_list_t const &list) {
  token_data_list_t result;
  result.reserve(list.size());
  for (auto const &d : list) {
    token_data_t data;
    data.name = d.name;
    data.tradeType = static_cast<trade_type_e>(d.tradeType);
    data.baseAsset = d.baseAsset;
    data.quoteAsset = d.quoteAsset;
    result.push_back(std::move(data));
  }
  return result;
}

token_owned_list_t
dbOwnedTokenListToBtOwnedToken(db_owned_token_list_t const &list,
                               db_token_list_t const &tokenList) {
  token_owned_list_t result;
  result.reserve(list.size());
  for (auto const &d : list) {
    auto iter = std::find_if(
        tokenList.cbegin(), tokenList.cend(),
        [tkID = d.tokenID](db_token_t const &a) { return a.tokenID == tkID; });
    if (iter == tokenList.cend())
      continue;
    token_owned_list_t::value_type data;
    data.tokenName = iter->name;
    data.amountInUse = d.amountInUse;
    data.amountAvailable = d.amountAvailable;
    result.push_back(std::move(data));
  }
  return result;
}

order_list_t dbOrderListToBtOrderList(db_user_order_list_t const &list,
                                      db_token_list_t const &tokenList) {
  order_list_t result;
  result.reserve(list.size());
  for (auto const &d : list) {
    auto iter =
        std::find_if(tokenList.cbegin(), tokenList.cend(),
                     [tkID = d.tokenID](db_token_list_t::value_type const &a) {
                       return a.tokenID == tkID;
                     });
    if (iter == tokenList.cend())
      continue;
    order_data_t data;
    data.tokenName = iter->name;
    data.leverage = d.leverage;
    data.orderID = d.orderID;
    data.market = static_cast<trade_market_e>(d.market);
    data.priceLevel = d.priceLevel;
    data.quantity = d.quantity;
    data.side = static_cast<trade_side_e>(d.side);
    data.type = static_cast<trade_type_e>(d.type);
    data.userID = d.userID;

    result.push_back(std::move(data));
  }
  return result;
}

trade_list_t dbTradeListToBtTradeList(db_trade_data_list_t const &list,
                                      db_token_list_t const &tokenList) {
  trade_list_t result;
  result.reserve(list.size());

  for (auto const &d : list) {
    auto iter = std::find_if(
        tokenList.cbegin(), tokenList.cend(),
        [tkID = d.tokenID](db_token_t const &a) { return a.tokenID == tkID; });
    if (iter == tokenList.cend())
      continue;
    trade_data_t data;
    data.amountPerPiece = d.amountPerPiece;
    data.quantityExecuted = d.quantityExec;
    data.orderID = d.orderID;
    data.side = static_cast<trade_side_e>(d.side);
    data.tokenName = iter->name;
    data.tradeID = d.tradeID;

    result.push_back(std::move(data));
  }
  return result;
}
} // namespace adaptor
} // namespace backtesting
