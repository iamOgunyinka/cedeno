#pragma once

#include "database_connector.hpp"
#include "user_data.hpp"

namespace backtesting {
namespace adaptor {
std::vector<token_data_t> dbTokenListToBtTokenList(db_token_list_t const &);
token_owned_list_t
dbOwnedTokenListToBtOwnedToken(db_owned_token_list_t const &,
                               db_token_list_t const &tokenList);
order_list_t dbOrderListToBtOrderList(db_user_order_list_t const &,
                                      db_token_list_t const &tokenList);
trade_list_t dbTradeListToBtTradeList(db_trade_data_list_t const &,
                                      db_token_list_t const &tokenList);
} // namespace adaptor
} // namespace backtesting
