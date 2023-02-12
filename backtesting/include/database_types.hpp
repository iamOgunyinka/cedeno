#pragma once

#ifdef BT_USE_WITH_DB

#include <string>
#include <vector>

namespace backtesting {

struct db_token_t {
  int tradeType = 0;
  std::string name;
  std::string baseAsset;
  std::string quoteAsset;
};
using db_token_list_t = std::vector<db_token_t>;

struct db_user_asset_t {
  std::string tokenName;
  double amountInUse = 0.0;
  double amountAvailable = 0.0;
  int databaseID = 0;
  int ownerID = 0;
};
using db_user_asset_list_t = std::vector<db_user_asset_t>;

struct db_user_t {
  int userID = 0;
};

struct db_user_order_t {
  int orderID = 0;
  int userID = 0;
  int side = 0;
  int type = 0;
  int market = 0;
  int status = 0;
  double quantity = 0.0;
  double priceLevel = 0.0;
  double leverage = 1.0;
  std::string symbol;
};
using db_user_order_list_t = std::vector<db_user_order_t>;

struct db_trade_data_t {
  int tradeID = 0;
  int orderID = 0;
  int userID = 0;
  int side = 0;
  int tradeType = 0;
  int status = 0;
  time_t eventTime = 0;
  double quantityExec = 0.0;
  double amountPerPiece = 0.0;
  std::string symbol;
};
using db_trade_data_list_t = std::vector<db_trade_data_t>;

} // namespace backtesting

#endif // BT_USE_WITH_DB
