#include "py_wrapper.hpp"
#include "bookticker.hpp"
#include "callbacks.hpp"
#include "candlestick_data.hpp"
#include "entry_point.hpp"
#include "indicator_data.hpp"

#include <pybind11/complex.h>
#include <pybind11/functional.h>

namespace py = pybind11;

extern bool isRunning;

namespace backtesting {
double currentPrice(std::string const &, trade_type_e const,
                    trade_side_e const);
bool createBTInstanceFromConfigFile(std::string const &);
bool endGlobalBTInstance();
bool startGlobalBTInstance(std::function<void()>, std::function<void()>
#ifdef BT_USE_WITH_INDICATORS
                           ,
                           indicator_callback_t
#endif
);
} // namespace backtesting

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
      .value("partially_filled", backtesting::order_status_e::partially_filled)
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

#ifdef BT_USE_WITH_INDICATORS
  py::class_<indicators::inf_BWFS_t>(m, "BwfsInfo")
      .def(py::init<>())
      .def_readonly("ticksIn", &indicators::inf_BWFS_t::ticks_in)
      .def_readonly("ticksOut", &indicators::inf_BWFS_t::ticks_out)
      .def_readonly("qtyIn", &indicators::inf_BWFS_t::qty_in)
      .def_readonly("qtyOut", &indicators::inf_BWFS_t::qty_out)
      .def_readonly("avgIn", &indicators::inf_BWFS_t::avrg_in)
      .def_readonly("avgOut", &indicators::inf_BWFS_t::avrg_out)
      .def_readonly("ticksInOut", &indicators::inf_BWFS_t::ticks_in_out)
      .def_readonly("qtyInOut", &indicators::inf_BWFS_t::qty_in_out)
      .def_readonly("buyerVsSeller", &indicators::inf_BWFS_t::buyer_vs_seller);

  py::class_<indicators::inf_ema_t>(m, "EmaInfo")
      .def(py::init<>())
      .def_readonly("price", &indicators::inf_ema_t::price);

  py::class_<indicators::inf_sma_t>(m, "SmaInfo")
      .def(py::init<>())
      .def_readonly("price", &indicators::inf_sma_t::price);

  py::class_<indicators::inf_macd_t>(m, "MacdInfo")
      .def(py::init<>())
      .def_readonly("price", &indicators::inf_macd_t::price);

  py::class_<indicators::inf_wma_t>(m, "WmaInfo")
      .def(py::init<>())
      .def_readonly("price", &indicators::inf_wma_t::price);

  py::class_<indicators::inf_atr_t>(m, "AtrInfo")
      .def(py::init<>())
      .def_readonly("price", &indicators::inf_atr_t::price);

  py::class_<indicators::inf_sar_t>(m, "SarInfo")
      .def(py::init<>())
      .def_readonly("priceUp", &indicators::inf_sar_t::price_up)
      .def_readonly("priceDown", &indicators::inf_sar_t::price_down)
      .def_readonly("status", &indicators::inf_sar_t::status);

  py::class_<indicators::inf_t>(m, "IndicatorInfo")
      .def(py::init<>())
      .def_readonly("cab", &indicators::inf_t::cab)
      .def_readonly("ema", &indicators::inf_t::ema)
      .def_readonly("sma", &indicators::inf_t::sma)
      .def_readonly("macd", &indicators::inf_t::macd)
      .def_readonly("wma", &indicators::inf_t::wma)
      .def_readonly("atr", &indicators::inf_t::atr)
      .def_readonly("sar", &indicators::inf_t::sar);

  py::class_<backtesting::timeframe_info_t>(m, "TimeframeData")
      .def(py::init<>())
      .def_readonly("isClosed", &backtesting::timeframe_info_t::isClosed)
      .def("__len__",
           [](backtesting::timeframe_info_t const &a) {
             return a.dataMap.size();
           })
      .def("__getitem__", [](backtesting::timeframe_info_t &self,
                             std::string const &str) { return self[str]; })
      .def(
          "__iter__",
          [](backtesting::timeframe_info_t const &a) {
            return py::make_key_iterator(a.dataMap.cbegin(), a.dataMap.cend());
          },
          py::keep_alive<0, 1>());
#endif

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
      .def_readonly("liquidationPrice",
                    &backtesting::position_t::liquidationPrice)
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
               &backtesting::user_data_t::createFuturesMarketOrder))
      .def("closePosition", &backtesting::user_data_t::closePosition)
      .def("closeAllPositions", &backtesting::user_data_t::closeAllPositions)
      .def("openQuickPosition", &backtesting::user_data_t::openQuickPosition);

  py::class_<backtesting_t>(m, "Backtesting")
      .def_static("instance",
                  [](backtesting::configuration_t config) {
                    return newBTInstance(std::move(config));
                  })
      .def("run", &backtesting_t::run);

  py::class_<backtesting::configuration_t>(m, "AppConfig")
      .def(py::init<>())
      .def_readwrite("trade", &backtesting::configuration_t::tradeType)
      .def_readwrite("symbols", &backtesting::configuration_t::tokenList)
      .def_readwrite("path", &backtesting::configuration_t::rootDir)
      .def_readwrite("dateStart", &backtesting::configuration_t::dateFromStr)
      .def_readwrite("dateEnd", &backtesting::configuration_t::dateToStr)
      .def_readwrite("verbose", &backtesting::configuration_t::verbose)
      .def_readwrite("klineConfig", &backtesting::configuration_t::klineConfig)
      .def_readwrite("booktickerConfig",
                     &backtesting::configuration_t::bookTickerConfig)
      .def_readwrite("futuresMakerFee",
                     &backtesting::configuration_t::futuresMakerFee)
      .def_readwrite("futuresTakerFee",
                     &backtesting::configuration_t::futuresTakerFee)
      .def_readwrite("spotMakerFee",
                     &backtesting::configuration_t::spotMakerFee)
      .def_readwrite("spotTakerFee",
                     &backtesting::configuration_t::spotTakerFee)
#ifdef BT_USE_WITH_DB
      .def_readwrite("dbConfigFilename",
                     &backtesting::configuration_t::dbConfigFilename)
      .def_readwrite("dbLaunchType",
                     &backtesting::configuration_t::dbLaunchType)
#endif

#ifdef BT_USE_WITH_INDICATORS
      .def_readwrite("indicatorConfig",
                     &backtesting::configuration_t::indicatorConfig)
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

  m.def("getCurrentPrice",
        [](std::string const &symbol, backtesting::trade_type_e const tt,
           backtesting::trade_side_e const side) {
          return backtesting::currentPrice(symbol, tt, side);
        });

  m.def("loadConfigFile", [](std::string const &filename) {
    return backtesting::createBTInstanceFromConfigFile(filename);
  });

#ifdef BT_USE_WITH_INDICATORS
  m.def(
      "startSimulation",
      [](std::function<void()> onStart, std::function<void()> onEnd,
         backtesting::indicator_callback_t onTick) {
        return backtesting::startGlobalBTInstance(onStart, onEnd, onTick);
      },
      py::arg("onStart") = nullptr, py::arg("onEnd") = nullptr,
      py::arg("onTick") = nullptr);

#else
  m.def(
      "startSimulation",
      [](std::function<void()> onStart, std::function<void()> onEnd) {
        return backtesting::startGlobalBTInstance(onStart, onEnd);
      },
      py::arg("onStart") = nullptr, py::arg("onEnd") = nullptr);
#endif

  m.def("endSimulation", [] { return backtesting::endGlobalBTInstance(); });

  m.def("cancelOrder", [](int64_t const orderID) {
    auto user = backtesting::getGlobalUser();
    if (!user)
      return false;
    return user->cancelOrderWithID(orderID);
  });

  m.def("createSpotLimitOrder",
        [](std::string const &base, std::string const &quote,
           double const price, double const quantity,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createSpotLimitOrder(base, quote, price, quantity, side);
        });

  m.def("createSpotLimitOrder",
        [](std::string const &tokenName, double const price,
           double const quantity,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createSpotLimitOrder(tokenName, price, quantity, side);
        });

  m.def("createSpotMarketOrder",
        [](std::string const &base, std::string const &quote,
           double const amountOrQtyToSpend,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createSpotMarketOrder(base, quote, amountOrQtyToSpend,
                                             side);
        });

  m.def("createSpotMarketOrder",
        [](std::string const &tokenName, double const amountOrQtyToSpend,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createSpotMarketOrder(tokenName, amountOrQtyToSpend,
                                             side);
        });

  m.def("createFuturesLimitOrder",
        [](std::string const &base, std::string const &quote,
           double const price, double const quantity,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createFuturesLimitOrder(base, quote, price, quantity,
                                               side);
        });

  m.def("createFuturesLimitOrder",
        [](std::string const &tokenName, double const price,
           double const quantity,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createFuturesLimitOrder(tokenName, price, quantity,
                                               side);
        });

  m.def("createFuturesMarketOrder",
        [](std::string const &base, std::string const &quote,
           double const amountOrQtyToSpend,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createFuturesMarketOrder(base, quote, amountOrQtyToSpend,
                                                side);
        });

  m.def("createFuturesMarketOrder",
        [](std::string const &tokenName, double const amountOrQtyToSpend,
           backtesting::trade_side_e const side) -> int64_t {
          auto user = backtesting::getGlobalUser();
          if (!user)
            return -1;
          return user->createFuturesMarketOrder(tokenName, amountOrQtyToSpend,
                                                side);
        });

  m.def("closePosition", [](std::string const &tokenName) {
    auto user = backtesting::getGlobalUser();
    if (!user)
      return false;
    return user->closePosition(tokenName);
  });

  m.def("closeAllPositions", [] {
    auto user = backtesting::getGlobalUser();
    if (!user)
      return false;
    return user->closeAllPositions();
  });

  m.def("openQuickPosition", [](std::string const &symbol, double const size,
                                backtesting::trade_side_e const side) {
    auto user = backtesting::getGlobalUser();
    if (!user)
      return false;
    return user->openQuickPosition(symbol, size, side);
  });

  m.def("isRunning", [] { return isRunning; });
}
