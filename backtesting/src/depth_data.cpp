#include <boost/asio/io_context.hpp>

#include "callbacks.hpp"
#include "container.hpp"
#include "depth_data.hpp"
#include "futures_order_book.hpp"
#include "matching_engine.hpp"
#include "spot_order_book.hpp"

namespace backtesting {
internal_token_data_t *getTokenWithName(std::string const &tokenName,
                                        trade_type_e const tradeType);

std::vector<global_order_book_t> global_order_book_t::globalOrderBooks{};

void processDepthStream(trade_map_td &tradeMap) {
  auto &globalOrderBooks = global_order_book_t::globalOrderBooks;

  globalOrderBooks.clear();
  auto sorter = [](token_map_td &map, char const *str) mutable
      -> std::optional<data_streamer_t<depth_data_t>> {
    if (auto iter = map.find(str); iter != map.end()) {
      auto &list = iter->second;
      std::sort(
          list.begin(), list.end(),
          [](std::filesystem::path const &a, std::filesystem::path const &b) {
            return std::filesystem::last_write_time(a) <
                   std::filesystem::last_write_time(b);
          });
      return data_streamer_t<depth_data_t>(list);
    }
    return std::nullopt;
  };

  auto getTradeSymbol = [](std::string const &name, trade_type_e const tt) {
    auto symbol = getTokenWithName(name, tt);
    if (!symbol)
      throw std::runtime_error("Invalid trade symbol");
    return symbol;
  };

  auto ioContext = std::make_shared<boost::asio::io_context>();
  double const futuresTakerFee = 1.0;
  double const futuresMakerFee = 1.0;

  for (auto &[tokenName, value] : tradeMap) {
    global_order_book_t d;
    d.spot = nullptr;
    d.futures = nullptr;
    d.tokenName = utils::toUpperString(tokenName);
    if (auto spotStreamer = sorter(value, SPOT); spotStreamer.has_value()) {
      auto symbol = getTradeSymbol(tokenName, trade_type_e::spot);
      d.spot.reset(
          new spot_order_book_t(*ioContext, std::move(*spotStreamer), symbol));
    }
    if (auto futuresStreamer = sorter(value, FUTURES);
        futuresStreamer.has_value()) {
      auto symbol = getTradeSymbol(tokenName, trade_type_e::futures);
      d.futures.reset(
          new futures_order_book_t(*ioContext, std::move(*futuresStreamer),
                                   symbol, futuresMakerFee, futuresTakerFee));
    }

    if (d.futures || d.spot)
      globalOrderBooks.push_back(std::move(d));
  }

  auto &newTradesDelegate = trade_signal_handler_t::GetTradesDelegate();
  auto &newDepthDelegate = depth_signal_handler_t::GetDepthDelegate();

  for (auto &orderBook : globalOrderBooks) {
    if (orderBook.futures) {
      orderBook.futures->NewDepthObtained.Connect(newDepthDelegate);
      orderBook.futures->NewTradesCreated.Connect(newTradesDelegate);
      orderBook.futures->run();
    }
    if (orderBook.spot) {
      orderBook.spot->NewDepthObtained.Connect(newDepthDelegate);
      orderBook.spot->NewTradesCreated.Connect(newTradesDelegate);
      orderBook.spot->run();
    }
  }

  std::thread{[=] { ioContext->run(); }}.detach();
}

bool depth_data_t::depthMetaFromCSV(csv::CSVRow const &row,
                                    depth_data_t &data) {
  if (row.size() == 0 || row.size() != 8)
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
  std::string eventType;
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
  if (eventType == "A")
    data.asks.push_back({priceLevel, quantity});
  else
    data.bids.push_back({priceLevel, quantity});
  data.tokenName = eventType;
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
  auto const isFutures = order.type == trade_type_e::futures;
  if ((isFutures && !orderBook.futures) || (!isFutures && !orderBook.spot))
    return false;

  matching_engine::matchOrder(isFutures ? *orderBook.futures : *orderBook.spot,
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
    auto const isSpot = order.type == trade_type_e::spot;
    matching_engine::cancelOrder(isSpot ? *orderBook.spot : *orderBook.futures,
                                 order);
  }
  return true;
}

py_depth_data_list_t depthDataToPythonDepth(depth_data_t const &data) {
  py_depth_data_list_t result;
  result.reserve(data.asks.size() + data.bids.size());

  for (auto const &a : data.asks) {
    py_depth_data_t data;
    data.eventTime = data.eventTime;
    data.price = a.priceLevel;
    data.quantity = a.quantity;
    result.push_back(std::move(data));
  }

  for (auto const &b : data.bids) {
    py_depth_data_t data;
    data.type = 1;
    data.eventTime = data.eventTime;
    data.price = b.priceLevel;
    data.quantity = b.quantity;
    result.push_back(std::move(data));
  }

  return result;
}

depth_callback_map_t depthCallbackList{};
::utils::waitable_container_t<depth_data_t> depthDataList{};
} // namespace backtesting
