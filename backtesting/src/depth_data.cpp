#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "depth_data.hpp"

#include "callbacks.hpp"
#include "container.hpp"
#include "futures_order_book.hpp"
#include "matching_engine.hpp"
#include "signals.hpp"
#include "spot_order_book.hpp"
#include <memory>

#ifdef BT_USE_WITH_INDICATORS
#include "tick.hpp"
#endif

namespace backtesting {
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e tradeType);

std::vector<global_order_book_t> global_order_book_t::globalOrderBooks{};

template <typename Type>
std::optional<data_streamer_t<Type>> getFileReader(token_map_td &map,
                                                   char const *str) {
  if (auto iter = map.find(str); iter != map.end()) {
    auto &list = iter->second;
    std::sort(
        list.begin(), list.end(),
        [](std::filesystem::path const &a, std::filesystem::path const &b) {
          return std::filesystem::last_write_time(a) <
                 std::filesystem::last_write_time(b);
        });
    return data_streamer_t<Type>(list);
  }
  return std::nullopt;
}

#ifdef BT_USE_WITH_INDICATORS
void processDepthStream(std::shared_ptr<net::io_context> ioContext,
                        trade_map_td &depthMap, trade_map_td &tMap,
                        std::vector<std::vector<std::string>> &&config) {
#else
void processDepthStream(std::shared_ptr<net::io_context> ioContext,
                        trade_map_td &depthMap, trade_map_td &tMap) {
#endif
  auto &globalOrderBooks = global_order_book_t::globalOrderBooks;

  globalOrderBooks.clear();

  auto getTradeSymbol = [](std::string const &name, trade_type_e const tt) {
    auto symbol = getTokenWithName(name, tt);
    if (!symbol)
      throw std::runtime_error("Invalid trade symbol");
    return symbol;
  };

  for (auto &[tokenName, value] : tMap) {
    global_order_book_t d;
    d.spot = nullptr;
    d.futures = nullptr;
    d.tokenName = utils::toUpperString(tokenName);
    if (auto spotStreamer = getFileReader<depth_data_t>(value, SPOT);
        spotStreamer.has_value()) {
      auto tradeReader = getFileReader<reader_trade_data_t>(value, SPOT);
      auto symbol = getTradeSymbol(tokenName, trade_type_e::spot);
      d.spot = std::make_unique<spot_order_book_t>(
          *ioContext, std::move(*spotStreamer), std::move(tradeReader), symbol);
    }
    if (auto futuresStreamer = getFileReader<depth_data_t>(value, FUTURES);
        futuresStreamer.has_value()) {
      auto tradeReader = getFileReader<reader_trade_data_t>(value, FUTURES);
      auto symbol = getTradeSymbol(tokenName, trade_type_e::futures);
      d.futures = std::make_unique<futures_order_book_t>(
          *ioContext, std::move(*futuresStreamer), std::move(tradeReader),
          symbol);
    }

    if (d.futures || d.spot)
      globalOrderBooks.push_back(std::move(d));
  }

  auto &newTradesDelegate = trade_signal_handler_t::GetTradesDelegate();
  auto &priceDelegate = signals_t::GetPriceDelegate();

  for (auto &orderBook : globalOrderBooks) {
    if (orderBook.futures) {
      orderBook.futures->NewTradesCreated.Connect(newTradesDelegate);
      orderBook.futures->NewMarketPrice.Connect(priceDelegate);
      orderBook.futures->run();
    }
    if (orderBook.spot) {
      orderBook.spot->NewTradesCreated.Connect(newTradesDelegate);
      orderBook.spot->run();
    }
  }

  std::thread{[=]() mutable {
#ifdef BT_USE_WITH_INDICATORS
    // call ::setIndicatorConfiguration only the indicators
    backtesting::order_book_base_t::setIndicatorConfiguration(
        std::move(config));
#endif
    ioContext->run();
  }}.detach();
}

bool depth_data_t::depthMetaFromCSV(csv::CSVRow const &row,
                                    depth_data_t &data) {
  if (row.empty() || row.size() != 8)
    return false;

  assert(row.size() == 8);
  std::string const eventType = row.begin()->get<std::string>();
  depth_meta_t d;
  d.priceLevel = getNumber<double>(row.begin() + 6);
  d.quantity = getNumber<double>(row.begin() + 7);

  if (auto const eventTime = getNumber<uint64_t>(row.begin() + 1);
      eventTime != data.eventTime)
    return false;

  if (eventType == "A")
    data.asks.push_back(std::move(d));
  else
    data.bids.push_back(std::move(d));
  return true;
}

depth_data_t depth_data_t::depthFromCSVRow(csv::CSVRow const &row) {
  csv::string_view eventType;
  double priceLevel = 0.0;
  double quantity = 0.0;
  depth_data_t data;

  assert(row.size() == 8);

  for (int i = 0; i < row.size(); ++i) {
    auto const iter = (row.begin() + i);
    if (i == 0) {
      eventType = iter->get_sv();
    } else if (i == 1) {
      data.eventTime = getNumber<uint64_t>(iter);
      //} else if (i == 2) {
      //  data.transactionTime = getNumber<uint64_t>(iter);
    } else if (i == 3) {
      data.firstUpdateID = getNumber<uint64_t>(iter);
    } else if (i == 4) {
      data.finalUpdateID = getNumber<uint64_t>(iter);
    } else if (i == 5) {
      data.finalStreamUpdateID = getNumber<uint64_t>(iter);
    } else if (i == 6) {
      priceLevel = getNumber<double>(iter);
    } else if (i == 7) {
      quantity = getNumber<double>(iter);
    }
  }
  if (eventType == csv::string_view("A"))
    data.asks.push_back({priceLevel, quantity});
  else
    data.bids.push_back({priceLevel, quantity});
  return data;
}

depth_data_t
depth_data_t::dataFromCSVStream(data_streamer_t<depth_data_t> &dataStreamer) {
  depth_data_t data;
  csv::CSVRow row;
  try {
    row = dataStreamer.getNextRow();
    if (row.empty())
      return depth_data_t{};
    data = depthFromCSVRow(row);
  } catch (std::exception const &) {
  }

  while (true) {
    try {
      row = dataStreamer.getNextRow();
      if (!depthMetaFromCSV(row, data)) {
        dataStreamer.putBack(std::move(row));
        break;
      }
    } catch (std::exception const &) {
    }
  }
  return data;
}

double orderBookBuyPrice(internal_token_data_t *const token) {
  auto &globalOrderBooks = global_order_book_t::globalOrderBooks;
  auto iter = std::find_if(globalOrderBooks.begin(), globalOrderBooks.end(),
                           [token](global_order_book_t &orderBook) {
                             return utils::isCaseInsensitiveStringCompare(
                                 orderBook.tokenName, token->name);
                           });
  if (iter == globalOrderBooks.end())
    return false;

  auto &orderBook = *iter;
  auto const isFutures = token->tradeType == trade_type_e::futures;
  if ((isFutures && !orderBook.futures) || (!isFutures && !orderBook.spot))
    return 0.0;
  auto &book = isFutures ? orderBook.futures : orderBook.spot;
  return book->currentBuyPrice();
}

double orderBookSellPrice(internal_token_data_t *const token) {
  auto &globalOrderBooks = global_order_book_t::globalOrderBooks;
  auto iter = std::find_if(globalOrderBooks.begin(), globalOrderBooks.end(),
                           [token](global_order_book_t &orderBook) {
                             return utils::isCaseInsensitiveStringCompare(
                                 orderBook.tokenName, token->name);
                           });
  if (iter == globalOrderBooks.end())
    return false;

  auto &orderBook = *iter;
  auto const isFutures = token->tradeType == trade_type_e::futures;
  if ((isFutures && !orderBook.futures) || (!isFutures && !orderBook.spot))
    return 0.0;
  auto &book = isFutures ? orderBook.futures : orderBook.spot;
  return book->currentSellPrice();
}

bool initiateOrder(order_data_t const &order) {
  auto &globalOrderBooks = global_order_book_t::globalOrderBooks;
  if (order.priceLevel < 0.0 || order.quantity < 0.0 || order.leverage < 1.0)
    return false;

  if (!order.token)
    return false;

  auto iter = std::find_if(globalOrderBooks.begin(), globalOrderBooks.end(),
                           [&order](global_order_book_t &orderBook) {
                             return utils::isCaseInsensitiveStringCompare(
                                 orderBook.tokenName, order.token->name);
                           });
  if (iter == globalOrderBooks.end())
    return false;

  auto &orderBook = *iter;
  bool const isFutures = order.type == trade_type_e::futures;
  if ((isFutures && !orderBook.futures) || (!isFutures && !orderBook.spot))
    return false;

  matching_engine::placeOrder(isFutures ? *orderBook.futures : *orderBook.spot,
                              order);
  return true;
}

bool cancelAllOrders(order_list_t const &orders) {
  auto &globalOrderBooks = global_order_book_t::globalOrderBooks;
  for (auto const &order : orders) {
    auto iter = std::find_if(globalOrderBooks.begin(), globalOrderBooks.end(),
                             [&order](global_order_book_t &orderBook) {
                               return utils::isCaseInsensitiveStringCompare(
                                   orderBook.tokenName, order.token->name);
                             });
    if (iter == globalOrderBooks.end())
      return false;
    auto &orderBook = *iter;
    bool const isSpot = order.type == trade_type_e::spot;
    matching_engine::cancelOrder(isSpot ? *orderBook.spot : *orderBook.futures,
                                 order);
  }
  return true;
}

py_depth_data_list_t depthDataToPythonDepth(depth_data_t const &data) {
  py_depth_data_list_t result;
  result.reserve(data.asks.size() + data.bids.size());

  for (auto const &a : data.asks) {
    py_depth_data_t d;
    d.eventTime = data.eventTime;
    d.price = a.priceLevel;
    d.quantity = a.quantity;
    result.push_back(std::move(d));
  }

  for (auto const &b : data.bids) {
    py_depth_data_t d;
    d.type = 1;
    d.eventTime = data.eventTime;
    d.price = b.priceLevel;
    d.quantity = b.quantity;
    result.push_back(std::move(d));
  }

  return result;
}

#ifdef BT_USE_WITH_INDICATORS
void scheduleCandlestickTask(unsigned long long startTime,
                             unsigned long long endTime) {
  std::vector<indicator_metadata_t *> indicators;
  auto &allOrderBooks = global_order_book_t::globalOrderBooks;

  indicators.reserve(allOrderBooks.size());

  for (auto &book : allOrderBooks) {
    kline_config_t config;
    config.symbol = book.tokenName;
    config.startTime = static_cast<time_t>(startTime);
    config.endTime = static_cast<time_t>(endTime);
    config.interval = data_interval_e::one_second;

    if (book.futures) {
      auto &indicatorMeta = book.futures->indicator();
      indicators.push_back(&indicatorMeta);

      config.tradeType = trade_type_e::futures;
      config.callback =
          [&indicatorMeta](backtesting::kline_data_list_t const &l) {
            indicatorMeta.indicator.process(l);
          };
      auto temp = config;
      (void)getContinuousKlineData(std::move(temp));
    };
    if (book.spot) {
      auto &indicatorMeta = book.spot->indicator();
      indicators.push_back(&indicatorMeta);

      config.tradeType = trade_type_e::spot;
      config.callback =
          [&indicatorMeta](backtesting::kline_data_list_t const &l) {
            indicatorMeta.indicator.process(l);
          };
      (void)getContinuousKlineData(std::move(config));
    }
  }

  if (!indicators.empty())
    ticker_t::instance()->addIndicators(indicators);
}
#endif

depth_callback_map_t depthCallbackList{};
::utils::waitable_container_t<depth_data_t> depthDataList{};
} // namespace backtesting
