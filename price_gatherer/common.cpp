#include "common.hpp"
#include <rapidjson/document.h>

namespace binance {
char const *const url_t::spotHostName = "stream.binance.com";
char const *const url_t::futuresHostName = "fstream.binance.com";
char const *const url_t::spotPortNumber = "9443";
char const *const url_t::futuresPortNumber = "443";

std::string toLowerString(std::string const &s) {
  std::string temp;
  temp.reserve(s.size());
  for (auto c : s)
    temp.push_back(std::tolower(c));
  return temp;
}

std::string toUpperString(std::string const &s) {
  std::string temp;
  temp.reserve(s.size());
  for (auto c : s)
    temp.push_back(std::toupper(c));
  return temp;
}

std::optional<candlestick_data_t> parseCandleStickData(char const *str,
                                                       size_t const size) {
#ifdef _MSC_VER
#undef GetObject
#endif

  rapidjson::Document d;
  d.Parse(str, size);

  if (!d.IsObject())
    return std::nullopt;

  auto const jsonObject = d.GetObject();
  auto const dataIter = jsonObject.FindMember("data");
  if (dataIter == jsonObject.MemberEnd() || !dataIter->value.IsObject())
    return std::nullopt;
  auto const dataObject = dataIter->value.GetObject();
  auto const dataTypeIter = dataObject.FindMember("e");
  if (dataTypeIter == dataObject.MemberEnd() || !dataTypeIter->value.IsString())
    return std::nullopt;
  auto const dataType = dataTypeIter->value.GetString();
  if (strcmp(dataType, "kline") != 0)
    return std::nullopt;
  auto const dataStreamIter = dataObject.FindMember("k");
  if (dataStreamIter == dataObject.MemberEnd() ||
      !dataStreamIter->value.IsObject())
    return std::nullopt;

  auto const dataStreamObject = dataStreamIter->value.GetObject();
  candlestick_data_t data;
  data.eventTime = dataObject.FindMember("E")->value.GetInt64();
  data.tokenName = dataStreamObject.FindMember("s")->value.GetString();
  data.interval = dataStreamObject.FindMember("i")->value.GetString();
  data.startTime = dataStreamObject.FindMember("t")->value.GetInt64();
  data.closeTime = dataStreamObject.FindMember("T")->value.GetInt64();
  data.firstTradeID = dataStreamObject.FindMember("f")->value.GetInt64();
  data.lastTradeID = dataStreamObject.FindMember("L")->value.GetInt64();
  data.openPrice = dataStreamObject.FindMember("o")->value.GetString();
  data.closePrice = dataStreamObject.FindMember("c")->value.GetString();
  data.highPrice = dataStreamObject.FindMember("h")->value.GetString();
  data.lowPrice = dataStreamObject.FindMember("l")->value.GetString();
  data.baseAssetVolume = dataStreamObject.FindMember("v")->value.GetString();
  data.numberOfTrades = dataStreamObject.FindMember("n")->value.GetInt();
  data.klineIsClosed = dataStreamObject.FindMember("x")->value.GetBool();
  data.quoteAssetVolume = dataStreamObject.FindMember("q")->value.GetString();
  data.tbBaseAssetVolume = dataStreamObject.FindMember("V")->value.GetString();
  data.tbQuoteAssetVolume = dataStreamObject.FindMember("Q")->value.GetString();
  return data;
}

std::optional<std::string> currentTimeToString(time_type_e const t) {
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning(disable : 4996)
#endif

  try {
    std::time_t current_time = std::time(nullptr);
    auto const tm_t = std::localtime(&current_time);
    if (!tm_t) {
      return std::nullopt;
    }
    std::string output((std::size_t)32, (char)'\0');
    auto const format = (t == time_type_e::date ? "%Y_%m_%d" : "%H_%M_%S");
    auto const string_length =
        std::strftime(output.data(), output.size(), format, tm_t);
    if (string_length) {
      output.resize(string_length);
      return output;
    }
  } catch (std::exception const &) {
  }
  return std::nullopt;
}

} // namespace binance
