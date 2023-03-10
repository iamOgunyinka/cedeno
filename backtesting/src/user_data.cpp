#include "user_data.hpp"
#include "global_data.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <thread>

namespace backtesting {
namespace utils {
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
} // namespace utils

int64_t getOrderNumber(); // defined in order_book.cpp
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e const tradeType) {
  auto &globalRtData = global_data_t::instance();
  for (auto &token : globalRtData.allTokens) {
    if ((tradeType == token.tradeType) &&
        utils::isCaseInsensitiveStringCompare(token.name, tokenName))
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
  for (auto &asset : m_assets)
    if (utils::isCaseInsensitiveStringCompare(asset.tokenName, name))
      return &asset;
  return nullptr;
}

void user_data_t::setLeverage(double const leverage_) {
  if (!m_openPositions.empty())
    throw std::runtime_error("cannot set leverage when a position is open");
  m_leverage = std::clamp(leverage_, 1.0, 125.0);
}

void user_data_t::calculateLiquidationPrice(position_t &pos,
                                            trade_market_e const marketType,
                                            trade_side_e const side) {
  double const closeCommission = futuresTakerFee;
  double const openCommission =
      marketType == trade_market_e::limit ? futuresMakerFee : futuresTakerFee;
  double liquidationPrice = 0.0;

  if (pos.side == trade_side_e::long_) {
    // LiqPrice(LONG) = EntryPrice * (1 - (Qty / Leverage - Qty *
    // (OpenCommission + CloseCommission)) / Qty)
    liquidationPrice =
        pos.entryPrice * (1 - (pos.size / pos.leverage -
                               pos.size * (openCommission + closeCommission)) /
                                  pos.size);
  } else {
    // LiqPrice(SHORT) = EntryPrice * (1 + (Qty / Leverage - Qty *
    // (OpenCommission + CloseCommission)) / Qty)
    liquidationPrice =
        pos.entryPrice * (1 + (pos.size / pos.leverage -
                               pos.size * (openCommission + closeCommission)) /
                                  pos.size);
  }

  if (pos.liquidationPrice == 0.0 || pos.side != side)
    pos.liquidationPrice = liquidationPrice;
  else
    pos.liquidationPrice = (pos.liquidationPrice + liquidationPrice) / 2.0;
}

void user_data_t::OnNewTrade(trade_data_t const &trade) {
  if (trade.tradeType == trade_type_e::none)
    return;
  else if (trade.tradeType == trade_type_e::spot)
    return onNewSpotTrade(trade);
  return onNewFuturesTrade(trade);
}

void user_data_t::onNewFuturesTrade(trade_data_t const &trade) {
  auto findOrderIter =
      std::find_if(m_orders.begin(), m_orders.end(),
                   [orderID = trade.orderID](order_data_t const &order) {
                     return order.orderID == orderID;
                   });
  if (findOrderIter == m_orders.end()) { // there's a problem
    return spdlog::error("There was a problem, order not found");
  }

  auto iter =
      std::find_if(m_openPositions.begin(), m_openPositions.end(),
                   [token = findOrderIter->token](position_t const &pos) {
                     return pos.token == token;
                   });
  if (iter == m_openPositions.end()) { // new position
    position_t pos;
    pos.token = findOrderIter->token;
    pos.entryPrice = trade.amountPerPiece;
    pos.side = trade.side;
    pos.leverage = findOrderIter->leverage;
    pos.size = trade.quantityExecuted;
    pos.status = trade.status;
    calculateLiquidationPrice(pos, findOrderIter->market, findOrderIter->side);

    return m_openPositions.push_back(std::move(pos));
  }
  spdlog::info("OrderID: {}, Side: {}, PosSize: {}, TradeSize: {}",
               trade.orderID, (int)trade.side, iter->size,
               trade.quantityExecuted);
  if (iter->side == trade.side) { // buying or selling more
    double const newEntryPrice =
        ((iter->entryPrice * iter->size) +
         (trade.quantityExecuted * trade.amountPerPiece)) /
        (iter->size + trade.quantityExecuted);
    iter->size += trade.quantityExecuted;
    iter->entryPrice = newEntryPrice;
    iter->status = trade.status;
    calculateLiquidationPrice(*iter, findOrderIter->market,
                              findOrderIter->side);
  } else {
    auto const sizeExecuted = (std::min)(iter->size, trade.quantityExecuted);
    calculatePNL(trade.amountPerPiece, sizeExecuted, *iter);

    if (auto const difference = iter->size - trade.quantityExecuted;
        difference == 0.0) { // close position
      m_openPositions.erase(iter);
    } else if (difference > 0.0) { // partial sale
      iter->size -= trade.quantityExecuted;
      calculateLiquidationPrice(*iter, findOrderIter->market, iter->side);
    } else {
      m_openPositions.erase(iter);

      position_t pos;
      pos.token = findOrderIter->token;
      pos.entryPrice = trade.amountPerPiece;
      pos.side = trade.side;
      pos.leverage = findOrderIter->leverage;
      pos.size = std::abs(difference);
      pos.status = trade.status;
      calculateLiquidationPrice(pos, findOrderIter->market,
                                findOrderIter->side);
      return m_openPositions.push_back(std::move(pos));
    }
  }
}

void user_data_t::calculatePNL(double const currentPrice, double const qty,
                               position_t const &position) {
  auto asset = getUserAsset(position.token->quoteAsset);
  if (!asset)
    return;

  double profitAndLoss = 0.0;
  if (position.side == trade_side_e::long_) {
    profitAndLoss =
        (currentPrice - position.entryPrice) * qty * position.leverage;
  } else {
    profitAndLoss =
        (position.entryPrice - currentPrice) * qty * position.leverage;
  }
  asset->amountAvailable += profitAndLoss;
}

bool user_data_t::isActiveOrder(const order_data_t &order) {
  return order.status == order_status_e::partially_filled ||
         order.status == order_status_e::new_order;
}

void user_data_t::OnNoTrade(order_data_t const &order) {
  if (order.type == trade_type_e::none)
    return;

  auto userOrderIter =
      std::find_if(m_orders.begin(), m_orders.end(),
                   [orderID = order.orderID](order_data_t const &order) {
                     return orderID == order.orderID;
                   });
  if (userOrderIter == m_orders.end() || !isActiveOrder(*userOrderIter))
    return;
  userOrderIter->status = order_status_e::expired;
  if (order.type == trade_type_e::spot)
    return issueRefund(order);
  return issueFuturesRefund(order);
}

void user_data_t::onNewSpotTrade(trade_data_t const &trade) {
  if (isBuyOrSell(trade.tradeType, trade.side))
    m_trades.push_back(trade);

  auto userOrderIter =
      std::find_if(m_orders.begin(), m_orders.end(),
                   [orderID = trade.orderID](order_data_t const &order) {
                     return orderID == order.orderID;
                   });
  if (userOrderIter == m_orders.end()) // nothing to do here
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

void user_data_t::liquidatePosition(position_t const &position) {
  auto iter = std::find_if(m_openPositions.begin(), m_openPositions.end(),
                           [symbol = position.token](position_t const &pos) {
                             return pos.token == symbol;
                           });
  if (iter == m_openPositions.end())
    return;

  // auto const side = iter->side == trade_side_e::long_ ? trade_side_e::short_
  //                                                    : trade_side_e::long_;
  // auto order =
  //    createOrderImpl(iter->token, iter->size, 0.0, 1.0,
  //    trade_type_e::futures,
  //                    side, trade_market_e::market);
  // sendOrderToBook(std::move(order));
  m_openPositions.erase(iter);
}

bool user_data_t::hasFuturesTradableBalance(internal_token_data_t *const token,
                                            double &quantity,
                                            double const amountToSpend,
                                            double const leverage,
                                            trade_side_e const side) {
  auto const flipSide = (side == trade_side_e::long_) ? trade_side_e::short_
                                                      : trade_side_e::long_;
  double price = amountToSpend;
  if (quantity == 0.0) {
    price = currentPrice(token, flipSide);
    if (price == 0.0)
      return false;

    quantity = (amountToSpend * leverage) / price;
    spdlog::info("Amount to spend is: {}, Qty: {}, Price: {}", amountToSpend,
                 quantity, price);
  }

  double cost = (quantity * price) / leverage;
  auto asset = getUserAsset(token->quoteAsset);
  if (!asset || asset->amountAvailable < cost) {
    return false;
  }

  auto positionIter = std::find_if(
      m_openPositions.begin(), m_openPositions.end(),
      [token](position_t const &position) { return position.token == token; });
  if (positionIter == m_openPositions.end() || positionIter->side == side) {
    asset->amountAvailable -= cost;
    return true;
  }

  quantity = positionIter->size - quantity;
  if (quantity == 0.0) // a close position
    return true;

  quantity = std::abs(quantity);
  // another open position
  cost = (quantity * price) / leverage;
  if (cost < amountToSpend) {
    // get the latest price
    price = currentPrice(token, flipSide);
    quantity = (amountToSpend * leverage) / price;
    cost = (quantity * price) / leverage;
  }
  spdlog::info("Recalculated cost is: {}, Qty: {}, Price: {}", cost, quantity,
               price);
  if (asset->amountAvailable < cost)
    return false;
  asset->amountAvailable -= cost;
  return true;
}

bool user_data_t::hasTradableBalance(internal_token_data_t *const token,
                                     double &quantity, double const price,
                                     double const leverage,
                                     trade_side_e const side) {
  if (!(token && isBuyOrSell(token->tradeType, side)))
    return false;

  if (token->tradeType == trade_type_e::futures)
    return hasFuturesTradableBalance(token, quantity, price, leverage, side);

  double const lot = (quantity == 0.0 ? 1.0 : quantity) * price;
  std::string const &symbol =
      (side == trade_side_e::buy) ? token->quoteAsset : token->baseAsset;
  auto asset = getUserAsset(symbol);
  if (!asset || asset->amountAvailable < lot)
    return false;

  asset->amountAvailable -= lot;
  asset->amountInUse += lot;
  return true;
}

void user_data_t::issueFuturesRefund(order_data_t const &order) {
  if (auto asset = getUserAsset(order.token->quoteAsset); asset) {
    double const cost = (order.quantity * order.priceLevel) / order.leverage;
    asset->amountAvailable += cost;
  }
}

void user_data_t::issueCancelledRefund(wallet_asset_t &asset,
                                       order_data_t const &order) {
  if (isActiveOrder(order)) {
    auto const lot = order.priceLevel * order.quantity;
    asset.amountAvailable += lot;
    asset.amountInUse -= lot;
  }
}

void user_data_t::issueRefund(order_data_t const &order) {
  if (isActiveOrder(order)) {
    auto const lot = order.quantity * order.priceLevel;
    std::string const &symbol = (order.side == trade_side_e::buy)
                                    ? order.token->quoteAsset
                                    : order.token->baseAsset;
    auto asset = getUserAsset(symbol);
    asset->amountAvailable += lot;
    asset->amountInUse -= lot;
  }
}

std::optional<order_data_t>
user_data_t::getLimitOrder(std::string const &tokenName, double const quantity,
                           double const price, double const leverage,
                           trade_side_e const side, trade_type_e const type) {
  auto token = getTokenWithName(tokenName, type);
  double qty = quantity;
  if (!(token && hasTradableBalance(token, qty, price, leverage, side)))
    return std::nullopt;

  return createOrderImpl(token, qty, price, leverage, type, side,
                         trade_market_e::limit);
}

std::optional<order_data_t> user_data_t::getMarketOrder(
    std::string const &tokenName, double const amountOrQuantityToSpend,
    double const leverage, trade_side_e const side, trade_type_e const type) {
  if (!isBuyOrSell(type, side))
    return std::nullopt;
  auto token = getTokenWithName(tokenName, type);
  double qty = 0.0;
  if (!(token && hasTradableBalance(token, qty, amountOrQuantityToSpend,
                                    leverage, side)))
    return std::nullopt;

  return createOrderImpl(token, qty, amountOrQuantityToSpend, leverage, type,
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

  m_orders.push_back(*order);
  if (bool const isSuccess = initiateOrder(*order); isSuccess) {
    orderNumber = order->orderID;
  } else {
    issueRefund(*order);
    m_orders.pop_back();
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
  auto order = getMarketOrder(tokenName, amountOrQtyToSpend, 1.0, side,
                              trade_type_e::spot);
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
  auto order = getLimitOrder(tokenName, quantity, price, m_leverage, side,
                             trade_type_e::futures);
  return sendOrderToBook(std::move(order));
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
  auto order = getMarketOrder(tokenName, amountOrQtyToSpend, m_leverage, side,
                              trade_type_e::futures);
  return sendOrderToBook(std::move(order));
}

bool user_data_t::cancelOrderWithID(uint64_t const orderID) {
  order_list_t cancelledOrders;
  auto iter = m_orders.end();
  do {
    iter = std::find_if(m_orders.begin(), m_orders.end(),
                        [orderID](order_data_t const &order) {
                          return order.orderID == orderID;
                        });
    if (m_orders.end() != iter && isActiveOrder(*iter)) {
      iter->status = order_status_e::pending_cancel;
      cancelledOrders.push_back(*iter);
    }
  } while (iter != m_orders.end());
  if (cancelledOrders.empty())
    return false;
  return cancelAllOrders(cancelledOrders);
}

} // namespace backtesting
