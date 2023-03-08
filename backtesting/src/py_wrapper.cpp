#include "py_wrapper.hpp"
#include "arguments_parser.hpp"
#include "bookticker.hpp"
#include "callbacks.hpp"
#include "candlestick_data.hpp"

#include <pybind11/complex.h>
#include <pybind11/functional.h>

namespace py = pybind11;
namespace backtesting
{
  double currentPrice(std::string const &, trade_type_e const);
}

std::optional<backtesting::user_data_t> findUserByID(int64_t userID) {
  if (userID < 0)
    return std::nullopt;

  auto const &users = global_data_t::instance().allUserAccounts;
  auto iter =
      std::find_if(users.cbegin(), users.cend(), [userID](auto const &user) {
        return user->m_userID == userID;
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
      .value("long", backtesting::trade_side_e::long_)
      .value("short", backtesting::trade_side_e::short_)
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

  py::enum_<backtesting::data_interval_e>(m, "DataInterval")
      .value("one_second", backtesting::data_interval_e::one_second)
      .value("one_minute", backtesting::data_interval_e::one_minute)
      .value("three_minutes", backtesting::data_interval_e::three_minutes)
      .value("five_minutes", backtesting::data_interval_e::five_minutes)
      .value("fifteen_minutes", backtesting::data_interval_e::fifteen_minutes)
      .value("thirty_minutes", backtesting::data_interval_e::thirty_minutes)
      .value("one_hour", backtesting::data_interval_e::one_hour)
      .value("two_hours", backtesting::data_interval_e::two_hours)
      .value("four_hours", backtesting::data_interval_e::four_hours)
      .value("six_hours", backtesting::data_interval_e::six_hours)
      .value("twelve_hours", backtesting::data_interval_e::twelve_hours)
      .value("one_day", backtesting::data_interval_e::one_day)
      .value("three_days", backtesting::data_interval_e::three_days)
      .value("one_week", backtesting::data_interval_e::one_week)
      .value("one_month", backtesting::data_interval_e::one_month);

  py::class_<backtesting::bktick_data_t>(m, "BooktickerData")
      .def(py::init<>())
      .def_readonly("ts", &backtesting::bktick_data_t::ts)
      .def_readonly("bestBidPrice", &backtesting::bktick_data_t::bestBidPrice)
      .def_readonly("bestBidQty", &backtesting::bktick_data_t::bestBidQty)
      .def_readonly("symbol", &backtesting::bktick_data_t::symbol)
      .def_readonly("bestAskPrice", &backtesting::bktick_data_t::bestAskPrice)
      .def_readonly("bestAskQty", &backtesting::bktick_data_t::bestAskQty);

  py::class_<backtesting::kline_data_t>(m, "KlineData")
      .def(py::init<>())
      .def_readonly("ts", &backtesting::kline_data_t::ts)
      .def_readonly("ntrades", &backtesting::kline_data_t::ntrades)
      .def_readonly("openPrice", &backtesting::kline_data_t::openPrice)
      .def_readonly("closePrice", &backtesting::kline_data_t::closePrice)
      .def_readonly("highPrice", &backtesting::kline_data_t::highPrice)
      .def_readonly("lowPrice", &backtesting::kline_data_t::lowPrice)
      .def_readonly("baseVolume", &backtesting::kline_data_t::baseVolume)
      .def_readonly("quoteVolume", &backtesting::kline_data_t::quoteVolume);

  py::class_<backtesting::kline_config_t>(m, "KlineConfig")
      .def(py::init<>())
      .def_readwrite("symbol", &backtesting::kline_config_t::symbol)
      .def_readwrite("tradeType", &backtesting::kline_config_t::tradeType)
      .def_readwrite("interval", &backtesting::kline_config_t::interval)
      .def_readwrite("startTime", &backtesting::kline_config_t::startTime)
      .def_readwrite("endTime", &backtesting::kline_config_t::endTime)
      .def_readwrite("maxLimit", &backtesting::kline_config_t::limit)
      .def("setCallback", [](backtesting::kline_config_t &self,
                             backtesting::kline_callback_t cb) {
        if (!cb)
          throw std::runtime_error("invalid callback passed to kline_config_t");
        self.callback = cb;
      });

  py::class_<backtesting::bktick_config_t>(m, "BooktickerConfig")
      .def(py::init<>())
      .def_readwrite("symbols", &backtesting::bktick_config_t::symbols)
      .def_readwrite("tradeType", &backtesting::bktick_config_t::tradeType)
      .def_readwrite("startTime", &backtesting::bktick_config_t::startTime)
      .def_readwrite("endTime", &backtesting::bktick_config_t::endTime)
      .def("setCallback", [](backtesting::bktick_config_t &self,
                             backtesting::bktick_callback_t cb) {
        if (!cb)
          throw std::runtime_error(
              "invalid callback passed to bookticker_config");
        self.callback = cb;
      });

  py::class_<backtesting::wallet_asset_t>(m, "Asset")
      .def(py::init<std::string const &, double const>())
      .def_readonly("inUse", &backtesting::wallet_asset_t::amountInUse)
      .def_property("available",
                    &backtesting::wallet_asset_t::getAvailableAmount,
                    &backtesting::wallet_asset_t::setAvailableAmount)
      .def_property("name", &backtesting::wallet_asset_t::getTokenName,
                    &backtesting::wallet_asset_t::setTokenName);

  py::class_<backtesting::py_depth_data_t>(m, "DepthData")
      .def(py::init<>())
      .def_readonly("ts", &backtesting::py_depth_data_t::eventTime)
      .def_readonly("type", &backtesting::py_depth_data_t::type)
      .def_readonly("price", &backtesting::py_depth_data_t::price)
      .def_readonly("quantity", &backtesting::py_depth_data_t::quantity);

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

  py::class_<backtesting::position_t>(m, "Position")
      .def_readonly("entryPrice", &backtesting::position_t::entryPrice)
      .def_readonly("size", &backtesting::position_t::size)
      .def_readonly("leverage", &backtesting::position_t::leverage)
      .def_readonly("liquidationPrice", &backtesting::position_t::liquidationPrice)
      .def_readonly("side", &backtesting::position_t::side);

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
      .def_readonly("userID", &backtesting::user_data_t::m_userID)
      .def_readonly("trades", &backtesting::user_data_t::m_trades)
      .def_readonly("orders", &backtesting::user_data_t::m_orders)
      .def_readonly("assets", &backtesting::user_data_t::m_assets)
      .def_readonly("positions", &backtesting::user_data_t::m_openPositions)
      .def_property("leverage", &backtesting::user_data_t::getLeverage,
                    &backtesting::user_data_t::setLeverage)
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
               &backtesting::user_data_t::createSpotMarketOrder))
      .def("createFuturesLimitOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, std::string const &, double const,
               double const, backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createFuturesLimitOrder))
      .def("createFuturesLimitOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, double const, double const,
               backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createFuturesLimitOrder))
      .def("createFuturesMarketOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, std::string const &, double const,
               backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createFuturesMarketOrder))
      .def("createFuturesMarketOrder",
           static_cast<int64_t (backtesting::user_data_t::*)(
               std::string const &, double const,
               backtesting::trade_side_e const)>(
               &backtesting::user_data_t::createFuturesMarketOrder));

  py::class_<backtesting_t>(m, "Backtesting")
      .def_static("instance",
                  [](backtesting::configuration_t config) {
                    return newBTInstance(std::move(config));
                  })
      .def("run", &backtesting_t::run);

  py::class_<backtesting::configuration_t>(m, "AppConfig")
      .def(py::init<>())
      .def_readwrite("trades", &backtesting::configuration_t::tradeTypes)
      .def_readwrite("symbols", &backtesting::configuration_t::tokenList)
      .def_readwrite("path", &backtesting::configuration_t::rootDir)
      .def_readwrite("dateStart", &backtesting::configuration_t::dateFromStr)
      .def_readwrite("dateEnd", &backtesting::configuration_t::dateToStr)
      .def_readwrite("klineConfig", &backtesting::configuration_t::klineConfig)
      .def_readwrite("booktickerConfig",
                     &backtesting::configuration_t::bookTickerConfig)
#ifdef BT_USE_WITH_DB
      .def_readwrite("dbConfigFilename",
                     &backtesting::configuration_t::dbConfigFilename)
      .def_readwrite("dbLaunchType",
                     &backtesting::configuration_t::dbLaunchType)
#endif
      ;

  m.def("sendOrder", &backtesting::initiateOrder);
  m.def("findUserByID", &findUserByID);
  m.def("addUser", [](backtesting::wallet_asset_list_t assets) {
    return findUserByID(global_data_t::newUser(std::move(assets)));
  });

  m.def("registerTradesCallback", [](backtesting::trade_type_e const tt,
                                     backtesting::recent_trades_callback_t cb) {
    return backtesting::registerTradesCallback(tt, cb, false);
  });

  m.def("registerAggTradesCallback",
        [](backtesting::trade_type_e const tt,
           backtesting::aggregate_trades_callback_t cb) {
          return backtesting::registerTradesCallback(tt, cb, false);
        });

  m.def("registerDepthCallback", [](backtesting::trade_type_e const tt,
                                    backtesting::depth_event_callback_t cb) {
    return backtesting::registerDepthCallback(tt, cb, false);
  });

  m.def(
      "getDiscreteKline", [](backtesting::kline_config_t config) -> auto {
        if (config.callback)
          config.callback = nullptr;
        return backtesting::getDiscreteKlineData(std::move(config));
      });

  m.def("getContinuousKline", [](backtesting::kline_config_t config) {
    if (!config.callback)
      return false;
    return backtesting::getContinuousKlineData(std::move(config));
  });

  m.def(
      "getBookticker", [](backtesting::bktick_config_t config) -> auto {
        if (config.callback)
          config.callback = nullptr;
        return backtesting::getDiscreteBTickerData(std::move(config));
      });

  m.def("getContinuousBookTicker", [](backtesting::bktick_config_t config) {
    if (!config.callback)
      return false;
    return backtesting::getContinuousBTickerData(std::move(config));
  });

  m.def("getCurrentPrice", [](std::string const &symbol, backtesting::trade_type_e const tt)
  {
    return backtesting::currentPrice(symbol, tt);
  });
}
