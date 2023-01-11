#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace backtesting {

enum class trade_type_e : int {
  none,
  spot,
  futures,
};

enum class trade_side_e : int {
  none,
  sell, // short
  buy,  // long
  cancel,
};

enum class trade_market_e : int {
  none,
  limit,
  market,
};
using market_type_e = trade_market_e;

enum class order_result_e : int {
  accepted,
  rejected,
};

struct token_data_t {
  uint64_t tokenID = 0; // optional
  std::string name;
  std::string baseAsset;
  std::string quoteAsset;
  trade_type_e tradeType = trade_type_e::none;
};
using token_data_list_t = std::vector<token_data_t>;

struct user_asset_t {
  std::string tokenName;
  double amountInUse = 0.0;
  double amountAvailable = 0.0;
  trade_type_e tradeType = trade_type_e::none;
};
using user_asset_list_t = std::vector<user_asset_t>;

struct order_data_t {
  std::string tokenName;
  uint64_t orderID = 0;
  uint64_t userID = 0;
  double quantity = 0.0;
  double priceLevel = 0.0;
  double leverage = 1.0;
  trade_side_e side = trade_side_e::none;
  trade_type_e type = trade_type_e::none;
  trade_market_e market = trade_market_e::none;
};
using order_list_t = std::vector<order_data_t>;

struct trade_data_t {
  std::string tokenName;
  uint64_t tradeID = 0;
  uint64_t orderID = 0;
  uint64_t eventTime = 0;
  double quantityExecuted = 0.0;
  double amountPerPiece = 0.0;
  trade_side_e side = trade_side_e::none;
};
using trade_list_t = std::vector<trade_data_t>;

struct user_data_t {
  uint64_t userID = 0;
  trade_list_t trades;
  order_list_t orders;
  user_asset_list_t assets;

  std::optional<order_data_t>
  createOrder(std::string const &tokenName, double const quantity,
              double const price, double const leverage = 1.0,
              trade_type_e const type = trade_type_e::spot,
              trade_side_e const side = trade_side_e::buy,
              trade_market_e const market = trade_market_e::limit);
  std::optional<order_data_t>
  createOrder(uint64_t const &tokenID, double const quantity,
              double const price, double const leverage = 1.0,
              trade_side_e const side = trade_side_e::buy,
              trade_market_e const market = trade_market_e::limit);
  uint64_t getUserID() const { return userID; }
  order_list_t getOrders() const { return orders; }
  trade_list_t getTrades() const { return trades; }
  user_asset_list_t getAssets() const { return assets; }

private:
  order_data_t createOrderImpl(std::string const &tokenName,
                               double const quantity, double const price,
                               double const leverage, trade_type_e const type,
                               trade_side_e const side,
                               trade_market_e const market);
};

using user_data_list_t = std::vector<user_data_t>;

bool initiateOrder(order_data_t const &order);
} // namespace backtesting
