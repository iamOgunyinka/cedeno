#ifdef BT_USE_WITH_DB

#include "adaptor.hpp"
#include <algorithm>

namespace backtesting {
namespace utils {
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
}

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
    data.tokenID = d.tokenID;
    result.push_back(std::move(data));
  }
  return result;
}

db_user_asset_list_t
btUserAssetsToDbUserAssets(uint64_t const userID,
                           user_asset_list_t const &userAssets,
                           token_data_list_t const &tokenList) {
  db_user_asset_list_t dbAssets;
  dbAssets.reserve(userAssets.size());
  for (auto const &a : userAssets) {
    db_user_asset_list_t::value_type asset;
    asset.ownerID = userID;
    asset.amountAvailable = a.amountAvailable;
    asset.amountInUse = a.amountInUse;
    auto iter = std::find_if(tokenList.cbegin(), tokenList.cend(),
                             [&a](token_data_t const &data) {
                               return utils::isCaseInsensitiveStringCompare(
                                          data.name, a.tokenName) &&
                                      (a.tradeType == data.tradeType);
                             });
    if (iter == std::cend(tokenList))
      throw std::runtime_error("Invalid token");
    asset.tokenID = iter->tokenID;
    dbAssets.push_back(std::move(asset));
  }
  return dbAssets;
}

user_asset_list_t
dbUserAssetsToBtUserAssets(db_user_asset_list_t const &dbAssets,
                           db_token_list_t const &tokenList) {
  user_asset_list_t result;
  result.reserve(dbAssets.size());
  for (auto const &d : dbAssets) {
    auto iter = std::find_if(
        tokenList.cbegin(), tokenList.cend(),
        [tkID = d.tokenID](db_token_t const &a) { return a.tokenID == tkID; });
    if (iter == tokenList.cend())
      continue;
    user_asset_list_t::value_type userAsset;
    userAsset.tokenName = iter->name;
    userAsset.amountInUse = d.amountInUse;
    userAsset.amountAvailable = d.amountAvailable;
    result.push_back(std::move(userAsset));
  }
  return result;
}

order_list_t dbOrderListToBtOrderList(db_user_order_list_t const &list,
                                      db_token_list_t const &tokenList,
                                      user_data_t *user) {
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
    data.user = user;

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

#endif // BT_USE_WITH_DB
