#include "ticker_stream.hpp"
#include <rapidjson/document.h>

namespace binance {
struct ticker_data_t {
  uint64_t eventTime = 0;
  uint64_t statisticsOpenTime = 0;
  uint64_t statisticsCloseTime = 0;
  uint64_t firstTradeID = 0;
  uint64_t lastTradeID = 0;
  uint64_t numberOfTrades = 0;
  std::string tokenName;
  std::string priceChange;
  std::string priceChangePercent;
  std::string weightedAveragePrice;
  std::string lastPrice;
  std::string lastQuantity;
  std::string openPrice;
  std::string highPrice;
  std::string lowPrice;
  std::string totalTradedBaseAssetVolume;
  std::string totalTradedQuoteAssetVolume;
  std::string bestBidPrice;
  std::string bestBidQty;
  std::string bestAskPrice;
  std::string bestAskQty;
};

std::string
ticker_stream_t::getSubscriptionMessage(std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@ticker\"],\"id\": 40}";
}

std::optional<ticker_data_t> parseTickerStream(char const *const str,
                                               size_t const length) {
#ifdef _MSC_VER
#undef GetObject
#endif

  rapidjson::Document d;
  d.Parse(str, length);

  if (!d.IsObject())
    return std::nullopt;

  auto const jsonObject = d.GetObject();
  auto const dataIter = jsonObject.FindMember("data");
  if (dataIter == jsonObject.end() || !dataIter->value.IsObject())
    return std::nullopt;
  auto const streamTypeIter = jsonObject.FindMember("stream");
  if (streamTypeIter == jsonObject.end() || !streamTypeIter->value.IsString())
    return std::nullopt;
  auto const dataType = streamTypeIter->value.GetString();
  if (strstr(dataType, "@ticker") == nullptr)
    return std::nullopt;
  auto const dataObject = dataIter->value.GetObject();
  ticker_data_t data;
  data.eventTime = dataObject.FindMember("E")->value.GetUint64();
  data.statisticsOpenTime = dataObject.FindMember("O")->value.GetUint64();
  data.statisticsCloseTime = dataObject.FindMember("C")->value.GetUint64();
  data.firstTradeID = dataObject.FindMember("F")->value.GetUint64();
  data.lastTradeID = dataObject.FindMember("L")->value.GetUint64();
  data.numberOfTrades = dataObject.FindMember("n")->value.GetUint64();
  data.tokenName = dataObject.FindMember("s")->value.GetString();
  data.priceChange = dataObject.FindMember("p")->value.GetString();
  data.priceChangePercent = dataObject.FindMember("P")->value.GetString();
  data.weightedAveragePrice = dataObject.FindMember("w")->value.GetString();
  data.lastPrice = dataObject.FindMember("c")->value.GetString();
  data.lastQuantity = dataObject.FindMember("Q")->value.GetString();
  data.openPrice = dataObject.FindMember("o")->value.GetString();
  data.highPrice = dataObject.FindMember("h")->value.GetString();
  data.lowPrice = dataObject.FindMember("l")->value.GetString();

  if (auto const v = dataObject.FindMember("b"); v != dataObject.end())
    data.bestBidPrice = v->value.GetString();

  if (auto const v = dataObject.FindMember("B"); v != dataObject.end())
    data.bestBidQty = v->value.GetString();

  if (auto const v = dataObject.FindMember("a"); v != dataObject.end())
    data.bestAskPrice = v->value.GetString();

  if (auto const v = dataObject.FindMember("A"); v != dataObject.end())
    data.bestAskQty = v->value.GetString();

  data.totalTradedBaseAssetVolume =
      dataObject.FindMember("v")->value.GetString();
  data.totalTradedQuoteAssetVolume =
      dataObject.FindMember("q")->value.GetString();
  return data;
}

void ticker_stream_t::processResponse(char const *const str,
                                      size_t const length) {
  if (auto const optTicker = parseTickerStream(str, length);
      optTicker.has_value()) {
    auto const &ticker = *optTicker;
    auto &os = m_tradeMap.dataMap[ticker.tokenName];
    if (os.rewriteHeader()) {
      os.rewriteHeader(false);
      writeCSVHeader();
    }

    os.write(ticker.eventTime, ticker.statisticsOpenTime,
             ticker.statisticsCloseTime, ticker.firstTradeID,
             ticker.lastTradeID, ticker.numberOfTrades, ticker.priceChange,
             ticker.priceChangePercent, ticker.weightedAveragePrice,
             ticker.lastPrice, ticker.lastQuantity, ticker.openPrice,
             ticker.highPrice, ticker.lowPrice,
             ticker.totalTradedBaseAssetVolume,
             ticker.totalTradedQuoteAssetVolume, ticker.bestBidPrice,
             ticker.bestBidQty, ticker.bestAskPrice, ticker.bestAskQty);
    if (++m_flushInterval == 2'000) {
      m_flushInterval = 0;
      os.flush();
    }
  }
}

void ticker_stream_t::writeCSVHeader() {
  for (auto &[_, file] : m_tradeMap.dataMap) {
    if (file.isOpen()) {
      file.write("eventTime", "statisticsOpenTime", "statisticsCloseTime",
                 "firstTradeID", "lastTradeID", "numberOfTrades", "priceChange",
                 "priceChangePercent", "weightedAveragePrice", "lastPrice",
                 "lastQuantity", "openPrice", "highPrice", "lowPrice",
                 "totalTradedBaseAssetVolume", "totalTradedQuoteAssetVolume",
                 "bestBidPrice", "bestBidQty", "bestAskPrice", "bestAskQty");
    }
  }
}

// ==============================================================

std::string book_ticker_stream_t::getSubscriptionMessage(
    std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@bookTicker\"],\"id\": 50}";
}

struct book_ticker_data_t {
  std::string bestBidPrice;
  std::string bestBidQty;
  std::string bestAskPrice;
  std::string bestAskQty;
  std::string tokenName;
  uint64_t eventTime = 0;
  uint64_t transactionTime = 0;
  uint64_t orderBookUpdateID = 0;
};

std::optional<book_ticker_data_t> parseBookTicker(char const *const str,
                                                  size_t const length) {
#ifdef _MSC_VER
#undef GetObject
#endif

  rapidjson::Document d;
  d.Parse(str, length);

  if (!d.IsObject())
    return std::nullopt;

  auto const jsonObject = d.GetObject();
  auto const dataIter = jsonObject.FindMember("data");
  if (dataIter == jsonObject.end() || !dataIter->value.IsObject())
    return std::nullopt;
  auto const streamTypeIter = jsonObject.FindMember("stream");
  if (streamTypeIter == jsonObject.end() || !streamTypeIter->value.IsString())
    return std::nullopt;
  auto const dataType = streamTypeIter->value.GetString();
  if (strstr(dataType, "@bookTicker") == nullptr)
    return std::nullopt;

  auto const dataObject = dataIter->value.GetObject();
  book_ticker_data_t data;
  data.orderBookUpdateID = dataObject.FindMember("u")->value.GetUint64();
  data.tokenName = dataObject.FindMember("s")->value.GetString();
  data.bestBidPrice = dataObject.FindMember("b")->value.GetString();
  data.bestBidQty = dataObject.FindMember("B")->value.GetString();
  data.bestAskPrice = dataObject.FindMember("a")->value.GetString();
  data.bestAskQty = dataObject.FindMember("A")->value.GetString();

  if (auto v = dataObject.FindMember("E"); v != dataObject.end())
    data.eventTime = v->value.GetUint64();

  if (auto const v = dataObject.FindMember("T"); v != dataObject.end())
    data.transactionTime = v->value.GetUint64();

  return data;
}

void book_ticker_stream_t::processResponse(char const *const str,
                                           size_t const length) {
  if (auto const bookTicker = parseBookTicker(str, length);
      bookTicker.has_value()) {
    auto &os = m_tradeMap.dataMap[bookTicker->tokenName];
    if (os.rewriteHeader()) {
      os.rewriteHeader(false);
      writeCSVHeader();
    }

    os.write(bookTicker->orderBookUpdateID, bookTicker->bestBidPrice,
             bookTicker->bestBidQty, bookTicker->bestAskPrice,
             bookTicker->bestAskQty, bookTicker->eventTime,
             bookTicker->transactionTime);
    if (++m_flushInterval == 2'000) {
      m_flushInterval = 0;
      os.flush();
    }
  }
}

void book_ticker_stream_t::writeCSVHeader() {
  for (auto &[_, file] : m_tradeMap.dataMap) {
    if (file.isOpen()) {
      file.write("OrderBookUpdateID", "BestBidPrice", "BestBidQty",
                 "BestAskPrice", "BestAskQty", "EventTime", "TransactionTime");
    }
  }
}
} // namespace binance
