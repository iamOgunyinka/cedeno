#ifdef BT_USE_WITH_DB

#include "adaptor.hpp"
#include <algorithm>

namespace backtesting {
namespace utils {
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
}

namespace adaptor {

internal_token_data_t dbTokenToBtTokenData(db_token_t const &d) {
  internal_token_data_t data;
  data.name = d.name;
  data.tradeType = static_cast<trade_type_e>(d.tradeType);
  data.baseAsset = d.baseAsset;
  data.quoteAsset = d.quoteAsset;
  return data;
}

token_data_list_t dbTokenListToBtTokenList(db_token_list_t const &list) {
  token_data_list_t result;
  result.reserve(list.size());
  for (auto const &d : list)
    result.push_back(dbTokenToBtTokenData(d));
  return result;
}

db_user_asset_list_t
btUserAssetsToDbUserAssets(uint64_t const userID,
                           user_asset_list_t const &userAssets) {
  db_user_asset_list_t dbAssets;
  dbAssets.reserve(userAssets.size());
  for (auto const &a : userAssets) {
    db_user_asset_list_t::value_type asset;
    asset.ownerID = userID;
    asset.amountAvailable = a.amountAvailable;
    asset.amountInUse = a.amountInUse;
    asset.tokenName = a.tokenName;
    dbAssets.push_back(std::move(asset));
  }
  return dbAssets;
}

user_asset_list_t
dbUserAssetsToBtUserAssets(db_user_asset_list_t const &dbAssets) {
  user_asset_list_t result;
  result.reserve(dbAssets.size());
  for (auto const &d : dbAssets) {
    user_asset_list_t::value_type userAsset;
    userAsset.tokenName = d.tokenName;
    userAsset.amountInUse = d.amountInUse;
    userAsset.amountAvailable = d.amountAvailable;
    result.push_back(std::move(userAsset));
  }
  return result;
}

order_list_t dbOrderListToBtOrderList(db_user_order_list_t const &list,
                                      token_data_list_t &btTokenList,
                                      user_data_t *user) {
  order_list_t result;
  result.reserve(list.size());
  for (auto const &d : list) {
    auto findIter = std::find_if(btTokenList.begin(), btTokenList.end(),
                                 [d](token_data_list_t::value_type const &a) {
                                   return utils::isCaseInsensitiveStringCompare(
                                       a.name, d.symbol);
                                 });
    if (findIter == btTokenList.cend())
      continue;
    order_data_t data;
    data.token = &(*findIter);
    data.leverage = d.leverage;
    data.orderID = d.orderID;
    data.market = static_cast<trade_market_e>(d.market);
    data.priceLevel = d.priceLevel;
    data.quantity = d.quantity;
    data.side = static_cast<trade_side_e>(d.side);
    data.type = static_cast<trade_type_e>(d.type);
    data.status = static_cast<order_status_e>(d.status);
    data.user = user;

    result.push_back(std::move(data));
  }
  return result;
}

trade_list_t dbTradeListToBtTradeList(db_trade_data_list_t const &list,
                                      token_data_list_t const &tokenList) {
  trade_list_t result;
  result.reserve(list.size());

  for (auto const &d : list) {
    auto iter = std::find_if(tokenList.cbegin(), tokenList.cend(),
                             [&d](token_data_list_t::value_type const &a) {
                               return utils::isCaseInsensitiveStringCompare(
                                   a.name, d.symbol);
                             });
    if (iter == tokenList.cend())
      continue;
    trade_data_t data;
    data.amountPerPiece = d.amountPerPiece;
    data.quantityExecuted = d.quantityExec;
    data.orderID = d.orderID;
    data.tradeType = static_cast<trade_type_e>(d.tradeType);
    data.side = static_cast<trade_side_e>(d.side);
    data.tokenName = iter->name;
    data.tradeID = d.tradeID;
    data.eventTime = d.eventTime;
    data.status = static_cast<order_status_e>(d.status);
    result.push_back(std::move(data));
  }
  return result;
}
} // namespace adaptor
} // namespace backtesting

#endif // BT_USE_WITH_DB
