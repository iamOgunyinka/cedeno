#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "trades_data.hpp"

namespace backtesting {

struct internal_token_data_t {
  std::string name;
  std::string baseAsset;
  std::string quoteAsset;
  trade_type_e tradeType = trade_type_e::none;
};
using token_data_list_t = std::vector<internal_token_data_t>;

struct position_t {
  internal_token_data_t *token = nullptr;
  double entry_price = 0.0;
  double quantity = 0.0;
  double leverage = 0.0;
  double pnl = 0.0;
  trade_side_e side = trade_side_e::none;

  position_t() = default;
  position_t(double const entry_price_, double const qty_,
             double const leverage_, trade_side_e const side_)
      : entry_price(entry_price_), quantity(qty_), leverage(leverage_),
        side(side_) {}
  inline double value(double const current_price) const {
    return quantity * current_price;
  }

  inline double margin() const { return value(entry_price) / leverage; }
};
using position_list_t = std::vector<position_t>;

struct wallet_asset_t {
  double amountInUse = 0.0;
  double amountAvailable = 0.0;
  std::string tokenName;

  wallet_asset_t() = default;
  wallet_asset_t(std::string const &name, double const amount)
      : tokenName(name), amountAvailable(amount) {}
  double getAvailableAmount() const { return amountAvailable; }
  void setAvailableAmount(double const d) { amountAvailable = d; }

  std::string getTokenName() const { return tokenName; }
  void setTokenName(std::string const &name);
};
using wallet_asset_list_t = std::vector<wallet_asset_t>;

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

struct user_data_t {
  friend class order_book_base_t;

  uint64_t userID = 0;
  double leverage = 1.0;
  double m_makerFeeRate = 0.0;
  double m_takerFeeRate = 0.0;
  trade_list_t trades;
  order_list_t orders;
  wallet_asset_list_t assets;
  position_list_t openPositions;

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
  int64_t createFuturesLimitOrder(std::string const &base,
                                  std::string const &quote, double const price,
                                  double const quantity,
                                  trade_side_e const side);
  int64_t createFuturesLimitOrder(std::string const &tokenName,
                                  double const price, double const quantity,
                                  trade_side_e const side);
  int64_t createFuturesMarketOrder(std::string const &base,
                                   std::string const &quote,
                                   double const amountOrQtyToSpend,
                                   trade_side_e const side);
  int64_t createFuturesMarketOrder(std::string const &tokenName,
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
  void setLeverage(double const leverage_);
  double getLeverage() const { return leverage; }
  bool cancelOrderWithID(uint64_t const orderID);

private:
  // this function is called from the order book AND the orderbook only
  void OnNewTrade(trade_data_t const &trade);
  void OnNoTrade(order_data_t const &order);

  void noSpotTrade(order_data_t const &order);
  void noFuturesTrade(order_data_t const &order);
  void onNewSpotTrade(trade_data_t const &trade);
  void onNewFuturesTrade(trade_data_t const &trade);
  void issueRefund(order_data_t const &order);
  void issueCancelledRefund(wallet_asset_t &asset, order_data_t const &order);
  [[nodiscard]] bool isBuyOrSell(trade_type_e const tt,
                                 trade_side_e const side) const;
  [[nodiscard]] bool updateOpenPositions(order_data_t const &order);
  [[nodiscard]] int64_t sendOrderToBook(std::optional<order_data_t> &&order);
  [[nodiscard]] wallet_asset_t *getUserAsset(std::string const &name);
  [[nodiscard]] bool hasTradableBalance(internal_token_data_t const *const,
                                        trade_side_e const side,
                                        double const quantity,
                                        double const amount,
                                        double const leverage);
  [[nodiscard]] bool
  hasFuturesTradableBalance(internal_token_data_t const *const,
                            trade_side_e const side, double const quantity,
                            double const amount, double const leverage);
  [[nodiscard]] order_data_t
  createOrderImpl(internal_token_data_t *, double const quantity,
                  double const price, double const leverage,
                  trade_type_e const type, trade_side_e const side,
                  trade_market_e const market);
};

using user_data_list_t = std::vector<std::shared_ptr<user_data_t>>;

bool initiateOrder(order_data_t const &order);
bool cancelAllOrders(order_list_t const &orders);
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e const tradeType);
} // namespace backtesting
