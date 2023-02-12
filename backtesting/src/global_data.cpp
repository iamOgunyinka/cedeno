#include "global_data.hpp"
#include <algorithm>

#ifdef BT_USE_WITH_DB
#include "database_connector.hpp"

namespace backtesting {
namespace adaptor {
db_user_asset_list_t
btUserAssetsToDbUserAssets(uint64_t const userID,
                           user_asset_list_t const &userAssets);
}
} // namespace backtesting
#endif // BT_USE_WITH_DB

int64_t global_data_t::newUser(backtesting::spot_wallet_asset_list_t assets) {
  auto user = std::make_unique<backtesting::user_data_t>();
  auto &globalRtData = global_data_t::instance();
  int64_t userID = -1;

  using backtesting::utils::toUpperString;

  auto &validSymbols = globalRtData.validSymbols;
  for (auto &asset : assets) {
    backtesting::utils::trim(asset.tokenName);
    asset.tokenName = toUpperString(asset.tokenName);
    if (!validSymbols.empty() && (validSymbols.find(asset.tokenName) ==
                                  globalRtData.validSymbols.cend())) {
      throw std::runtime_error("Invalid symbol");
      return userID;
    }
  }
  user->assets = std::move(assets);

#ifdef BT_USE_WITH_DB
  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  if (!databaseConnector)
    return userID;

  do {
    auto const username = backtesting::utils::getRandomString(
        backtesting::utils::getRandomInteger());
    userID = databaseConnector->addNewUser(username);
  } while (userID < 0);

  user->userID = (uint64_t)userID;
  if (!databaseConnector->addUserAssets(
          backtesting::adaptor::btUserAssetsToDbUserAssets(user->userID,
                                                           user->assets)))
    return userID;
#else
  {
    auto iter = globalRtData.allUserAccounts.cend();
    do {
      auto const userID = backtesting::utils::getRandomInteger();
      iter = std::find_if(
          globalRtData.allUserAccounts.cbegin(),
          globalRtData.allUserAccounts.cend(),
          [userID](std::shared_ptr<backtesting::user_data_t> const &user) {
            return user->userID == userID;
          });
      user->userID = userID;
    } while (iter != globalRtData.allUserAccounts.cend());
  }
#endif

  globalRtData.allUserAccounts.push_back(std::move(user));
  return userID;
}

global_data_t &global_data_t::instance() {
  static global_data_t globalData;
  return globalData;
}
