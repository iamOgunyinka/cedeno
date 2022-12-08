#include "trade_stream.hpp"
#include <iostream>
#include <rapidjson/document.h>

namespace binance {
std::string
trade_stream_t::getSubscriptionMessage(std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@aggTrade\"],\"id\": 30}";
}

struct trade_stream_data_t {
  bool isBuyerMarketMaker = true;
  uint64_t eventTime = 0;
  uint64_t aggregateTradeID = 0;
  uint64_t firstTradeID = 0;
  uint64_t lastTradeID = 0;
  uint64_t tradeTime = 0;
  std::string price;
  std::string quantity;
  std::string tokenName;
};

std::optional<trade_stream_data_t> parseTradeStream(char const *const str,
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
  if (strstr(dataType, "@aggTrade") == nullptr)
    return std::nullopt;

  auto const dataObject = dataIter->value.GetObject();
  trade_stream_data_t data;
  data.isBuyerMarketMaker = dataObject.FindMember("m")->value.GetBool();
  data.eventTime = dataObject.FindMember("E")->value.GetUint64();
  data.aggregateTradeID = dataObject.FindMember("a")->value.GetUint64();
  data.firstTradeID = dataObject.FindMember("f")->value.GetUint64();
  data.lastTradeID = dataObject.FindMember("l")->value.GetUint64();
  data.tradeTime = dataObject.FindMember("T")->value.GetUint64();
  data.price = dataObject.FindMember("p")->value.GetString();
  data.quantity = dataObject.FindMember("q")->value.GetString();
  data.tokenName = dataObject.FindMember("s")->value.GetString();
  return data;
}

void trade_stream_t::processResponse(char const *const str,
                                     size_t const length) {
  if (auto const optTradeStream = parseTradeStream(str, length);
      optTradeStream.has_value()) {
    auto const &t = *optTradeStream;
    auto &os = m_tradeMap.dataMap[t.tokenName];
    os.write(t.eventTime, t.aggregateTradeID, t.firstTradeID, t.lastTradeID,
             t.tradeTime, t.price, t.quantity, t.isBuyerMarketMaker);
    if (++m_flushInterval == 20'000) {
      m_flushInterval = 0;
      os.flush();
    }
  }
}

void trade_stream_t::writeCSVHeader() {
  for (auto &[_, file] : m_tradeMap.dataMap) {
    if (file.isOpen()) {
      file.write("eventTime", "aggregateTradeID", "firstTradeID", "lastTradeID",
                 "tradeTime", "price", "quantity", "isBuyerMarketMaker");
    }
  }
}
} // namespace binance
