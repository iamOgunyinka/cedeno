#include "signals.hpp"
#include "global_data.hpp"
#include "user_data.hpp"
#include <thread>

namespace backtesting {

std::unordered_map<internal_token_data_t *, double> signals_t::latestPrices{};
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e const tradeType);

void liquidationOfPositionsImpl() {
  auto &users = global_data_t::instance().allUserAccounts;

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    if (users.empty())
      continue;

    for (auto &user : users) {
      if (user->m_openPositions.empty())
        continue;

    positionLoop:
      for (position_t &position : user->m_openPositions) {
        for (auto const &[symbol, currentPrice] : signals_t::latestPrices) {
          if (symbol == position.token &&
              ((position.side == trade_side_e::long_ &&
                position.liquidationPrice <= currentPrice) ||
               (position.side == trade_side_e::short_ &&
                position.liquidationPrice >= currentPrice))) {

            // liquidatePosition will remove the position specified and
            // invalidate the openPositions iterators, so it's better we
            // restart from the beginning again
            user->liquidatePosition(position);
            goto positionLoop;
          }
        }
      }
    }
  }
}

void signals_t::OnNewFuturesPrice(internal_token_data_t *token,
                                  double const newPrice) {
  if (token == nullptr || token->tradeType != trade_type_e::futures ||
      latestPrices[token] == newPrice)
    return;
  latestPrices[token] = newPrice;
}

signals_t::price_delegate_t &signals_t::GetPriceDelegate() {
  static std::unique_ptr<price_delegate_t> priceDelegate = nullptr;
  if (!priceDelegate) {
    priceDelegate.reset(new price_delegate_t{});
    priceDelegate->Bind(&OnNewFuturesPrice);
  }
  return *priceDelegate;
}

double currentPrice(internal_token_data_t *const token) {
  auto &prices = signals_t::latestPrices;
  if (auto iter = prices.find(token); iter != prices.end())
    return iter->second;
  return 0.0;
}

double currentPrice(std::string const &tokenName, trade_type_e const tt) {
  auto token = getTokenWithName(tokenName, tt);
  if (!token)
    return 0.0;
  return currentPrice(token);
}

} // namespace backtesting
