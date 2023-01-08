#include "global_data.hpp"
#include "database_connector.hpp"

extern global_data_t globalRtData;

namespace backtesting {
namespace adaptor {
db_user_asset_list_t
btUserAssetsToDbUserAssets(uint64_t const userID,
                           user_asset_list_t const &userAssets,
                           token_data_list_t const &tokenList);
}
} // namespace backtesting

bool global_data_t::newUser(backtesting::user_asset_list_t assets,
                            bool forceCreationIfUserExists) {
  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  if (!databaseConnector)
    return false;

  backtesting::user_data_t user;
  user.assets = std::move(assets);

  auto const username = backtesting::utils::getRandomString(
      backtesting::utils::getRandomInteger());
  user.userID = databaseConnector->addNewUser(username);
  if (user.userID < 0)
    return false;
  if (!databaseConnector->addUserAssets(
          backtesting::adaptor::btUserAssetsToDbUserAssets(
              user.userID, user.assets, globalRtData.allTokens)))
    return false;

  globalRtData.allUserAccounts.push_back(std::move(user));
  return true;
}
