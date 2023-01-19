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

enum class order_status_e : int {
  new_order,
  partially_filled,
  filled,
  cancelled,
  pending_cancel,
  rejected,
  expired,
};

struct internal_token_data_t {
  std::string name;
  std::string baseAsset;
  std::string quoteAsset;
  trade_type_e tradeType = trade_type_e::none;
};
using token_data_list_t = std::vector<internal_token_data_t>;

struct user_asset_t {
  double amountInUse = 0.0;
  double amountAvailable = 0.0;
  std::string tokenName;

  double getAvailableAmount() const { return amountAvailable; }
  void setAvailableAmount(double const d) { amountAvailable = d; }

  std::string getTokenName() const { return tokenName; }
  void setTokenName(std::string const &name);
};
using user_asset_list_t = std::vector<user_asset_t>;

struct user_data_t;
struct order_data_t {
  uint64_t orderID = 0;
  user_data_t *user = nullptr;
  internal_token_data_t *token = nullptr;
  double quantity = 0.0;
  double priceLevel = 0.0;
  double leverage = 1.0;
  trade_side_e side = trade_side_e::none;
  trade_type_e type = trade_type_e::none;
  trade_market_e market = trade_market_e::none;
  order_status_e status = order_status_e::new_order;
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
  trade_type_e tradeType = trade_type_e::none;
  order_status_e status = order_status_e::new_order;
};
using trade_list_t = std::vector<trade_data_t>;

struct user_data_t {
  uint64_t userID = 0;
  trade_list_t trades;
  order_list_t orders;
  user_asset_list_t assets;

  int64_t createSpotLimitOrder(std::string const &base,
                               std::string const &quote, double const price,
                               double const quantity, trade_side_e const side);
  int64_t createSpotLimitOrder(std::string const &tokenName, double const price,
                               double const quantity, trade_side_e const side);
  int64_t createSpotMarketOrder(std::string const &base,
                                std::string const &quote,
                                double const amountOrQtyToSpend,
                                trade_side_e const side);
  int64_t createSpotMarketOrder(std::string const &tokenName,
                                double const amountOrQtyToSpend,
                                trade_side_e const side);
  std::optional<order_data_t>
  getLimitOrder(std::string const &tokenName, double const quantity,
                double const price, double const leverage = 1.0,
                trade_side_e const side = trade_side_e::buy,
                trade_type_e const type = trade_type_e::spot);
  std::optional<order_data_t> getMarketOrder(
      std::string const &tokenName, double const amountOrQuantityToSpend,
      double const leverage = 1.0, trade_side_e const side = trade_side_e::buy,
      trade_type_e const tradeType = trade_type_e::spot);
  bool cancelOrderWithID(uint64_t const orderID);
  void OnNewTrade(trade_data_t const &trade);

private:
  inline bool isBuyOrSell(trade_side_e const side) const {
    return side == trade_side_e::buy || side == trade_side_e::sell;
  }

  void issueRefund(order_data_t const &order);
  int64_t sendOrderToBook(std::optional<order_data_t> &&order);
  user_asset_t *getUserAsset(std::string const &name);
  bool hasTradableBalance(internal_token_data_t const *const,
                          trade_side_e const side, double const quantity,
                          double const amount, double const leverage);
  order_data_t createOrderImpl(internal_token_data_t *, double const quantity,
                               double const price, double const leverage,
                               trade_type_e const type, trade_side_e const side,
                               trade_market_e const market);
};

using user_data_list_t = std::vector<std::shared_ptr<user_data_t>>;
using new_trades_callback_t = void (*)(backtesting::trade_list_t const &);
bool initiateOrder(order_data_t const &order);
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e const tradeType);
} // namespace backtesting
