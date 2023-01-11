#include "py_wrapper.hpp"

namespace py = pybind11;

extern std::map<int, std::vector<backtesting::new_trades_callback_t>>
    registeredCallbacks;

PYBIND11_MODULE(backtester, m) {
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
      .def_readwrite("type", &backtesting::order_data_t::type)
      .def_readwrite("userID", &backtesting::order_data_t::userID);

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

  py::class_<global_data_t>(m, "globals")
      .def_readonly("allTokens", &global_data_t::allTokens)
      .def("newUser",
           static_cast<bool (*)(backtesting::user_asset_list_t, bool)>(
               &global_data_t::newUser))
      .def("onNewTrades", [](backtesting::trade_type_e const tt,
                             backtesting::new_trades_callback_t callback) {
        registeredCallbacks[(int)tt].push_back(callback);
      });

  py::class_<token_data_t>(m, "token_data_t")
      .def_readwrite("tokenID", &token_data_t::tokenID)
      .def_readwrite("name", &token_data_t::name)
      .def_readwrite("baseAsset", &token_data_t::baseAsset)
      .def_readwrite("quoteAsset", &token_data_t::quoteAsset)
      .def_readwrite("tradeType", &token_data_t::tradeType);
}
