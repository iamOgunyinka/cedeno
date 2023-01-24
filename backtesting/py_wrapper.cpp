#include "py_wrapper.hpp"
#include "arguments_parser.hpp"

namespace py = pybind11;

std::optional<backtesting::user_data_t> findUserByID(int64_t userID) {
  if (userID < 0)
    return std::nullopt;

  auto const &users = global_data_t::instance().allUserAccounts;
  auto iter =
      std::find_if(users.cbegin(), users.cend(), [userID](auto const &user) {
        return user->userID == userID;
      });
  if (iter == users.cend())
    return std::nullopt;
  return *(*iter);
}

PYBIND11_MODULE(jbacktest, m) {
  using backtesting::internal_token_data_t;

  py::enum_<backtesting::trade_type_e>(m, "TradeType")
      .value("none", backtesting::trade_type_e::none)
      .value("spot", backtesting::trade_type_e::spot)
      .value("futures", backtesting::trade_type_e::futures);

  py::enum_<backtesting::trade_side_e>(m, "TradeSide")
      .value("none", backtesting::trade_side_e::none)
      .value("buy", backtesting::trade_side_e::buy)
      .value("sell", backtesting::trade_side_e::sell);

  py::enum_<backtesting::trade_market_e>(m, "TradeMarket")
      .value("none", backtesting::trade_market_e::none)
      .value("limit", backtesting::trade_market_e::limit)
      .value("market", backtesting::trade_market_e::market);

  py::enum_<backtesting::order_status_e>(m, "OrderStatus")
      .value("cancelled", backtesting::order_status_e::cancelled)
      .value("expired", backtesting::order_status_e::expired)
      .value("filled", backtesting::order_status_e::filled)
      .value("new_order", backtesting::order_status_e::new_order)
      .value("partiall_filled", backtesting::order_status_e::partially_filled)
      .value("pending_cancel", backtesting::order_status_e::pending_cancel)
      .value("rejected", backtesting::order_status_e::rejected);

  py::class_<backtesting::configuration_t>(m, "Configuration")
      .def(py::init<>())
      .def_readwrite("trades", &backtesting::configuration_t::tradeTypes)
      .def_readwrite("symbols", &backtesting::configuration_t::tokenList)
      .def_readwrite("path", &backtesting::configuration_t::rootDir)
      .def_readwrite("dateStart", &backtesting::configuration_t::dateFromStr)
      .def_readwrite("dateEnd", &backtesting::configuration_t::dateToStr)
#ifdef BT_USE_WITH_DB
      .def_readwrite("dbConfigFilename",
                     &backtesting::configuration_t::dbConfigFilename)
      .def_readwrite("dbLaunchType",
                     &backtesting::configuration_t::dbLaunchType)
#endif
      ;

  py::class_<backtesting::spot_wallet_asset_t>(m, "SpotWalletAsset")
      .def(py::init<std::string const &, double const>())
      .def_readonly("inUse", &backtesting::spot_wallet_asset_t::amountInUse)
      .def_property("available",
                    &backtesting::spot_wallet_asset_t::getAvailableAmount,
                    &backtesting::spot_wallet_asset_t::setAvailableAmount)
      .def_property("symbolName",
                    &backtesting::spot_wallet_asset_t::getTokenName,
                    &backtesting::spot_wallet_asset_t::setTokenName);

  py::class_<backtesting::order_data_t>(m, "OrderData")
      .def(py::init<>())
      .def_readonly("leverage", &backtesting::order_data_t::leverage)
      .def_readonly("market", &backtesting::order_data_t::market)
      .def_readonly("orderID", &backtesting::order_data_t::orderID)
      .def_readonly("price", &backtesting::order_data_t::priceLevel)
      .def_readonly("quantity", &backtesting::order_data_t::quantity)
      .def_readonly("side", &backtesting::order_data_t::side)
      .def_readonly("type", &backtesting::order_data_t::type)
      .def_readonly("status", &backtesting::order_data_t::status)
      .def_property_readonly("symbolName", [](backtesting::order_data_t &a) {
        return a.token != nullptr ? a.token->name : std::string{};
      });

  py::class_<backtesting::trade_data_t>(m, "TradeData")
      .def_readonly("symbolName", &backtesting::trade_data_t::tokenName)
      .def_readonly("tradeID", &backtesting::trade_data_t::tradeID)
      .def_readonly("orderID", &backtesting::trade_data_t::orderID)
      .def_readonly("eventTime", &backtesting::trade_data_t::eventTime)
      .def_readonly("type", &backtesting::trade_data_t::tradeType)
      .def_readonly("quantityExecuted",
                    &backtesting::trade_data_t::quantityExecuted)
      .def_readonly("amountPerPiece",
                    &backtesting::trade_data_t::amountPerPiece)
      .def_readonly("side", &backtesting::trade_data_t::side);

  py::class_<backtesting::user_data_t>(m, "UserData")
      .def_readonly("userID", &backtesting::user_data_t::userID)
      .def_readonly("trades", &backtesting::user_data_t::trades)
      .def_readonly("orders", &backtesting::user_data_t::orders)
      .def_readonly("assets", &backtesting::user_data_t::assets)
      .def("cancelOrder", &backtesting::user_data_t::cancelOrderWithID)
      .def("createSpotLimitOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, std::string const &, double const,
               double const, backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createSpotLimitOrder))
      .def("createSpotLimitOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, double const, double const,
               backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createSpotLimitOrder))
      .def("createSpotMarketOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, std::string const &, double const,
               backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createSpotMarketOrder))
      .def("createSpotMarketOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, double const,
               backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createSpotMarketOrder));

  py::class_<backtesting_t>(m, "Backtesting")
      .def_static("instance",
                  [](backtesting::configuration_t config) {
                    return newBTInstance(std::move(config));
                  })
      .def("run", &backtesting_t::run);

  m.def("sendOrder", &backtesting::initiateOrder);
  m.def("findUserByID", &findUserByID);
  m.def("addUser", [](backtesting::spot_wallet_asset_list_t assets) {
    return findUserByID(global_data_t::newUser(std::move(assets)));
  });
  m.def("registerNewTradesCallback", [](backtesting::trade_type_e const tt,
                                        backtesting::new_trades_callback_t cb) {
    return backtesting::registerNewTradesCallback(tt, cb, false);
  });
}
