#pragma once

#include "database_connector.hpp"
#include "user_data.hpp"

namespace backtesting {
namespace adaptor {
db_user_asset_list_t
btUserAssetsToDbUserAssets(uint64_t const userID,
                           user_asset_list_t const &userAssets,
                           token_data_list_t const &tokenList);

token_data_list_t dbTokenListToBtTokenList(db_token_list_t const &);
user_asset_list_t dbUserAssetsToBtUserAssets(db_user_asset_list_t const &,
                                             db_token_list_t const &tokenList);
order_list_t dbOrderListToBtOrderList(db_user_order_list_t const &,
                                      db_token_list_t const &tokenList);
trade_list_t dbTradeListToBtTradeList(db_trade_data_list_t const &,
                                      db_token_list_t const &tokenList);
} // namespace adaptor
} // namespace backtesting
