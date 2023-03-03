#include "user_data.hpp"
#include "global_data.hpp"
#include <algorithm>

namespace backtesting {
namespace utils {
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
} // namespace utils

int64_t getOrderNumber(); // defined in order_book.cpp
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e const tradeType) {
  auto &globalRtData = global_data_t::instance();
  for (auto &token : globalRtData.allTokens) {
    if (utils::isCaseInsensitiveStringCompare(token.name, tokenName) &&
        (tradeType == token.tradeType))
      return &token;
  }
  return nullptr;
}

void wallet_asset_t::setTokenName(std::string const &name) {
  tokenName = utils::trim_copy(name);
  if (tokenName.empty())
    throw std::logic_error("invalid token name");
}

wallet_asset_t *user_data_t::getUserAsset(std::string const &name) {
  for (auto &asset : assets)
    if (utils::isCaseInsensitiveStringCompare(asset.tokenName, name))
      return &asset;
  return nullptr;
}

void user_data_t::setLeverage(double const leverage_) {
  leverage = std::clamp(leverage_, 1.0, 125.0);
}

void user_data_t::OnNewTrade(trade_data_t const &trade) {
  if (trade.tradeType == trade_type_e::none)
    return;
  else if (trade.tradeType == trade_type_e::spot)
    return OnNewSpotTrade(trade);
  return OnNewFuturesTrade(trade);
}

void user_data_t::OnNewFuturesTrade(trade_data_t const &trade) {
  // TODO:
  (void)trade;
}

void user_data_t::OnNewSpotTrade(trade_data_t const &trade) {
  if (isBuyOrSell(trade.tradeType, trade.side))
    trades.push_back(trade);

  auto userOrderIter =
      std::find_if(orders.begin(), orders.end(),
                   [orderID = trade.orderID](order_data_t const &order) {
                     return orderID == order.orderID;
                   });
  if (userOrderIter == orders.end()) // nothing to do here
    return;

  auto &order = *userOrderIter;
  auto const lot = trade.quantityExecuted * trade.amountPerPiece;
  order.status = trade.status;

  bool const isBuying = order.side == trade_side_e::buy;
  auto const &asset =
      isBuying ? order.token->quoteAsset : order.token->baseAsset;
  auto userAsset = getUserAsset(order.token->quoteAsset);
  if (!userAsset)
    throw std::logic_error("asset spent is not available");

  if (order.status == order_status_e::cancelled)
    return issueCancelledRefund(*userAsset, order);
  if (isBuying) {
    userAsset->amountAvailable += trade.quantityExecuted;
    userAsset->amountInUse -= lot;
  } else if (order.side == trade_side_e::sell) {
    userAsset->amountAvailable += lot;
    userAsset->amountInUse -= trade.quantityExecuted;
  }
}

bool user_data_t::isBuyOrSell(trade_type_e const tt,
                              trade_side_e const side) const {
  if (tt == trade_type_e::spot)
    return side == trade_side_e::buy || side == trade_side_e::sell;
  else if (tt == trade_type_e::futures)
    return side == trade_side_e::long_ || side == trade_side_e::short_;
  return false;
}

bool user_data_t::hasTradableBalance(internal_token_data_t const *const token,
                                     trade_side_e const side,
                                     double const quantity, double const price,
                                     double const leverage) {
  if (!(token && isBuyOrSell(token->tradeType, side)))
    return false;

  double const lot = quantity * price * leverage;
  std::string const &symbol =
      (side == trade_side_e::buy) ? token->quoteAsset : token->baseAsset;
  auto asset = getUserAsset(symbol);
  if (!asset || asset->amountAvailable < lot)
    return false;

  asset->amountAvailable -= lot;
  asset->amountInUse += lot;
  return true;
}

void user_data_t::issueCancelledRefund(wallet_asset_t &asset,
                                       order_data_t const &order) {
  auto const lot = order.priceLevel * order.quantity;
  asset.amountAvailable += lot;
  asset.amountInUse -= lot;
}

void user_data_t::issueRefund(order_data_t const &order) {
  auto const lot = order.quantity * order.priceLevel * order.leverage;
  std::string const &symbol = (order.side == trade_side_e::buy)
                                  ? order.token->quoteAsset
                                  : order.token->baseAsset;
  auto asset = getUserAsset(symbol);
  asset->amountAvailable += lot;
  asset->amountInUse -= lot;
}

std::optional<order_data_t>
user_data_t::getLimitOrder(std::string const &tokenName, double const quantity,
                           double const price, double const leverage,
                           trade_side_e const side, trade_type_e const type) {
  auto token = getTokenWithName(tokenName, type);
  if (!(token && hasTradableBalance(token, side, quantity, price, leverage)))
    return std::nullopt;

  return createOrderImpl(token, quantity, price, leverage, type, side,
                         trade_market_e::limit);
}

std::optional<order_data_t> user_data_t::getMarketOrder(
    std::string const &tokenName, double const amountOrQuantityToSpend,
    double const leverage, trade_side_e const side, trade_type_e const type) {
  if (!isBuyOrSell(type, side))
    return std::nullopt;

  auto token = getTokenWithName(tokenName, type);
  if (!(token && hasTradableBalance(token, side, 1.0, amountOrQuantityToSpend,
                                    leverage)))
    return std::nullopt;

  return createOrderImpl(token, 1.0, amountOrQuantityToSpend, leverage, type,
                         side, trade_market_e::market);
}

order_data_t user_data_t::createOrderImpl(
    internal_token_data_t *token, double const quantity, double const price,
    double const leverage, trade_type_e const tradeType,
    trade_side_e const side, trade_market_e const market) {
  order_data_t order;
  order.orderID = getOrderNumber();
  order.leverage = leverage;
  order.market = market;
  order.priceLevel = price;
  order.quantity = quantity;
  order.side = side;
  order.token = token;
  order.type = tradeType;
  order.user = this;
  order.status = order_status_e::new_order;
  return order;
}

int64_t user_data_t::sendOrderToBook(std::optional<order_data_t> &&order) {
  int64_t orderNumber = -1;

  if (!order)
    return orderNumber;

  if (bool const isSuccess = initiateOrder(*order); isSuccess) {
    orderNumber = order->orderID;
    orders.push_back(std::move(*order));
  } else {
    issueRefund(*order);
  }
  return orderNumber;
}

int64_t user_data_t::createSpotLimitOrder(std::string const &base,
                                          std::string const &quote,
                                          double const price,
                                          double const quantity,
                                          trade_side_e const side) {
  auto const fullTokenName = utils::toUpperString(base + quote);
  return createSpotLimitOrder(fullTokenName, price, quantity, side);
}

int64_t user_data_t::createSpotMarketOrder(std::string const &base,
                                           std::string const &quote,
                                           double const amountOrQtyToSpend,
                                           trade_side_e const side) {
  auto const tokenName = utils::toUpperString(base + quote);
  return createSpotMarketOrder(tokenName, amountOrQtyToSpend, side);
}

int64_t user_data_t::createSpotLimitOrder(std::string const &tokenName,
                                          double const price,
                                          double const quantity,
                                          trade_side_e const side) {
  // in the case of spot trading, leverage is always 1.0
  auto order = getLimitOrder(tokenName, quantity, price, 1.0, side);
  return sendOrderToBook(std::move(order));
}

int64_t user_data_t::createSpotMarketOrder(std::string const &tokenName,
                                           double const amountOrQtyToSpend,
                                           trade_side_e const side) {
  // in the case of spot trading, leverage is always 1.0
  auto order = getMarketOrder(tokenName, amountOrQtyToSpend, 1.0, side);
  return sendOrderToBook(std::move(order));
}

int64_t user_data_t::createFuturesLimitOrder(std::string const &base,
                                             std::string const &quote,
                                             double const price,
                                             double const quantity,
                                             trade_side_e const side) {
  auto const fullTokenName = utils::toUpperString(base + quote);
  return createFuturesLimitOrder(fullTokenName, price, quantity, side);
}

int64_t user_data_t::createFuturesLimitOrder(std::string const &tokenName,
                                             double const price,
                                             double const quantity,
                                             trade_side_e const side) {
  [[maybe_unused]] auto order = getLimitOrder(
      tokenName, quantity, price, leverage, side, trade_type_e::futures);
  // TODO:
  return -1;
  // return sendOrderToBook(std::move(order));
}

int64_t user_data_t::createFuturesMarketOrder(std::string const &base,
                                              std::string const &quote,
                                              double const amountOrQtyToSpend,
                                              trade_side_e const side) {
  auto const tokenName = utils::toUpperString(base + quote);
  return createFuturesMarketOrder(tokenName, amountOrQtyToSpend, side);
}

int64_t user_data_t::createFuturesMarketOrder(std::string const &tokenName,
                                              double const amountOrQtyToSpend,
                                              trade_side_e const side) {
  // TODO:
  return -1;
}

bool user_data_t::cancelOrderWithID(uint64_t const orderID) {
  order_list_t cancelledOrders;
  auto iter = orders.end();
  do {
    iter = std::find_if(orders.begin(), orders.end(),
                        [orderID](order_data_t const &order) {
                          return order.orderID == orderID;
                        });
    if (orders.end() != iter) {
      iter->status = order_status_e::pending_cancel;
      cancelledOrders.push_back(*iter);
    }
  } while (iter != orders.end());
  if (cancelledOrders.empty())
    return false;
  return cancelAllOrders(cancelledOrders);
}

} // namespace backtesting
