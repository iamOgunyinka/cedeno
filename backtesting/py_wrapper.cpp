#include "py_wrapper.hpp"
#include "arguments_parser.hpp"

namespace py = pybind11;

extern std::map<int, std::vector<backtesting::new_trades_callback_t>>
    registeredCallbacks;

PYBIND11_MODULE(jbacktest, m) {
  using backtesting::token_data_t;

  py::enum_<backtesting::trade_type_e>(m, "trade_type")
      .value("none", backtesting::trade_type_e::none)
      .value("spot", backtesting::trade_type_e::spot)
      .value("futures", backtesting::trade_type_e::futures);

  py::enum_<backtesting::trade_side_e>(m, "trade_side")
      .value("none", backtesting::trade_side_e::none)
      .value("buy", backtesting::trade_side_e::buy)
      .value("sell", backtesting::trade_side_e::sell)
      .value("cancel", backtesting::trade_side_e::cancel);

  py::enum_<backtesting::trade_market_e>(m, "trade_market")
      .value("none", backtesting::trade_market_e::none)
      .value("limit", backtesting::trade_market_e::limit)
      .value("market", backtesting::trade_market_e::market);

  py::enum_<backtesting::order_result_e>(m, "order_result")
      .value("accepted", backtesting::order_result_e::accepted)
      .value("rejected", backtesting::order_result_e::rejected);

  py::class_<backtesting::configuration_t>(m, "configuration")
      .def_readwrite("streams", &backtesting::configuration_t::streams)
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

  py::class_<backtesting::user_asset_t>(m, "user_asset")
      .def_readwrite("tokenName", &backtesting::user_asset_t::tokenName)
      .def_readwrite("amountInUse", &backtesting::user_asset_t::amountInUse)
      .def_readwrite("tokenName", &backtesting::user_asset_t::amountAvailable)
      .def_readwrite("tradeType", &backtesting::user_asset_t::tradeType);

  py::class_<backtesting::order_data_t>(m, "order_data")
      .def_readwrite("leverage", &backtesting::order_data_t::leverage)
      .def_readwrite("market", &backtesting::order_data_t::market)
      .def_readwrite("orderID", &backtesting::order_data_t::orderID)
      .def_readwrite("price", &backtesting::order_data_t::priceLevel)
      .def_readwrite("quantity", &backtesting::order_data_t::quantity)
      .def_readwrite("side", &backtesting::order_data_t::side)
      .def_readwrite("tokenName", &backtesting::order_data_t::tokenName)
      .def_readwrite("type", &backtesting::order_data_t::type);

  py::class_<backtesting::trade_data_t>(m, "trade_data")
      .def_readwrite("tokenName", &backtesting::trade_data_t::tokenName)
      .def_readwrite("tradeID", &backtesting::trade_data_t::tradeID)
      .def_readwrite("orderID", &backtesting::trade_data_t::orderID)
      .def_readwrite("eventTime", &backtesting::trade_data_t::eventTime)
      .def_readwrite("quantityExecuted",
                     &backtesting::trade_data_t::quantityExecuted)
      .def_readwrite("amountPerPiece",
                     &backtesting::trade_data_t::amountPerPiece)
      .def_readwrite("side", &backtesting::trade_data_t::side);

  py::class_<backtesting::user_data_t>(m, "user_data")
      .def_readwrite("userID", &backtesting::user_data_t::userID)
      .def_readwrite("trades", &backtesting::user_data_t::trades)
      .def_readwrite("orders", &backtesting::user_data_t::orders)
      .def_readwrite("assets", &backtesting::user_data_t::assets)
      .def("createOrder",
           static_cast<std::optional<backtesting::order_data_t> (
               backtesting::user_data_t::*)(
               std::string const &, double, double const, double const,
               backtesting::trade_type_e const, backtesting::trade_side_e const,
               backtesting::trade_market_e const)>(
               &backtesting::user_data_t::createOrder))
      .def("createOrder", static_cast<std::optional<backtesting::order_data_t> (
                              backtesting::user_data_t::*)(
                              uint64_t const, double const, double const,
                              double const, backtesting::trade_side_e const,
                              backtesting::trade_market_e const)>(
                              &backtesting::user_data_t::createOrder))
      .def("getUserID", &backtesting::user_data_t::getUserID)
      .def("getOrders", &backtesting::user_data_t::getOrders)
      .def("getTrades", &backtesting::user_data_t::getTrades)
      .def("getAssets", &backtesting::user_data_t::getAssets);

  py::class_<token_data_t>(m, "token")
      .def_readwrite("tokenID", &token_data_t::tokenID)
      .def_readwrite("name", &token_data_t::name)
      .def_readwrite("baseAsset", &token_data_t::baseAsset)
      .def_readwrite("quoteAsset", &token_data_t::quoteAsset)
      .def_readwrite("tradeType", &token_data_t::tradeType);

  py::class_<backtesting_t>(m, "backtesting")
      .def("parse",
           [](backtesting_t &a, std::vector<std::string> args) {
             std::vector<char *> csStrs;
             csStrs.reserve(args.size());
             for (auto &s : args)
               csStrs.push_back(const_cast<char *>(s.c_str()));
             return a.parse(csStrs.size(), csStrs.data());
           })
      .def_static("instance",
                  [](backtesting::configuration_t config) {
                    return newBTInstance(std::move(config));
                  })
      .def("run", &backtesting_t::run);

  m.def("allTokens", [] { return global_data_t::instance().allTokens; });
  m.def("registerNewTradesCallback",
        [](backtesting::trade_type_e tt,
           backtesting::new_trades_callback_t callback) {
          registeredCallbacks[(int)tt].push_back(callback);
        });
  m.def("addUser", &global_data_t::newUser);
  m.def("findUser",
        [](uint64_t userID) -> std::optional<backtesting::user_data_t> {
          auto const &users = global_data_t::instance().allUserAccounts;
          auto iter = std::find_if(
              users.cbegin(), users.cend(),
              [userID](auto const &user) { return user->userID == userID; });
          if (iter == users.cend())
            return std::nullopt;
          return *(*iter);
        });
}
