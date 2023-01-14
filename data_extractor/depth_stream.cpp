#include "depth_stream.hpp"
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/write.hpp>
#include <iostream>
#include <rapidjson/document.h>

namespace binance {
std::string
depth_stream_t::getSubscriptionMessage(std::string const &tokenName) const {
  return "{\"method\": \"SUBSCRIBE\", \"params\":["
         "\"" +
         tokenName + "@depth\"],\"id\": 80}";
}

std::optional<depth_data_t> parseDepthStream(char const *const str,
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
  if (strstr(dataType, "@depth") == nullptr)
    return std::nullopt;
  auto const dataObject = dataIter->value.GetObject();

  depth_data_t data;
  data.tokenName = toUpperString(dataObject.FindMember("s")->value.GetString());
  data.eventTime = dataObject.FindMember("E")->value.GetUint64();
  if (auto const v = dataObject.FindMember("T"); v != dataObject.end())
    data.transactionTime = v->value.GetUint64();
  data.firstUpdateID = dataObject.FindMember("U")->value.GetUint64();
  data.finalUpdateID = dataObject.FindMember("u")->value.GetUint64();

  if (auto const v = dataObject.FindMember("pu"); v != dataObject.end())
    data.finalStreamUpdateID = v->value.GetUint64();

  static auto bidAskDecoder =
      [](std::vector<depth_data_t::depth_meta_t> &result,
         rapidjson::GenericArray<false, rapidjson::Value> const &dataList) {
        result.reserve(dataList.Size());
        for (auto const &tempItem : dataList) {
          auto const item = tempItem.GetArray();
          if (item.Size() != 2)
            continue;
          depth_data_t::depth_meta_t d;
          d.priceLevel = std::stod(item[0].GetString());
          d.quantity = std::stod(item[1].GetString());
          result.push_back(std::move(d));
        }
      };
  bidAskDecoder(data.bids, dataObject.FindMember("b")->value.GetArray());
  bidAskDecoder(data.asks, dataObject.FindMember("a")->value.GetArray());
  return data;
}

void depth_stream_t::writeCSVBody(depth_data_t &&depth) {
  auto &os = m_tradeMap.dataMap[depth.tokenName];
  if (os.rewriteHeader())
    writeCSVHeader(os);

  for (auto const &a : depth.asks) {
    os.write("A", depth.eventTime, depth.transactionTime, depth.firstUpdateID,
             depth.finalUpdateID, depth.finalStreamUpdateID, a.priceLevel,
             a.quantity);
  }

  for (auto const &b : depth.bids) {
    os.write("B", depth.eventTime, depth.transactionTime, depth.firstUpdateID,
             depth.finalUpdateID, depth.finalStreamUpdateID, b.priceLevel,
             b.quantity);
  }
}

void depth_stream_t::processResponse(char const *const str,
                                     size_t const length) {
  if (auto optDepth = parseDepthStream(str, length); optDepth.has_value()) {
    m_dataList.append(std::move(*optDepth));
    if (m_isFirstRequest) {
      m_isFirstRequest = false;
      getDepthSnapshotNoAsync();
    }
  }
}

void depth_stream_t::writeCSVHeader(binance::locked_file_t &os) {
  if (os.isOpen())
    os.write("eventType", "eventTime", "transactionTime", "firstUpdateID",
             "finalUpdateID", "finalStreamUpdateID", "priceLevel", "quantity");
  os.rewriteHeader(false);
}

void depth_stream_t::startMonitoringThread() {
  std::thread([this] { startMonitoringThreadImpl(); }).detach();
}

void depth_stream_t::startMonitoringThreadImpl() {
  while (true) {
    auto tempData = m_dataList.get();
    writeCSVBody(std::move(tempData));
  }
}

void depth_stream_t::getDepthSnapshotNoAsync() {
  m_resolver.emplace(m_ioContext);
  std::string const prefix = m_tradeType == trade_type_e::futures ? "f" : "";
  auto const host = prefix + "api.binance.com";

  beast::ssl_stream<beast::tcp_stream> stream(m_ioContext, m_sslContext);
  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         net::error::get_ssl_category()};
    return;
  }

  auto const results = m_resolver->resolve(host, "443");
  m_resolver.reset();

  beast::get_lowest_layer(stream).connect(results);

  // Perform the SSL handshake
  stream.handshake(net::ssl::stream_base::client);

  auto const s = "/" + prefix + "api/v1/depth?symbol=";
  for (auto const &token : m_tokens) {
    auto const path = s + token.tokenName + "&limit=50";
    http::request<http::string_body> request{http::verb::get, path, 11};
    request.set(http::field::host, host);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::accept, "*/*");

    http::write(stream, request);

    m_readBuffer.emplace();
    // Declare a container to hold the response
    http::response<http::string_body> response;

    // Receive the HTTP response
    http::read(stream, *m_readBuffer, response);
    m_readBuffer.reset();

    auto const &body = response.body();

    if (auto data = parseDepthStream(body.data(), body.size()); data)
      m_dataList.append(std::move(*data));
  }
}

std::vector<std::string> fetchToken(net::io_context &ioContext,
                                    net::ssl::context &sslContext,
                                    trade_type_e const tradeType) {
  std::optional<net::ip::tcp::resolver> resolver;

  bool const isFutures = (tradeType == trade_type_e::futures);
  std::string const prefix = isFutures ? "f" : "";
  auto const host = prefix + "api.binance.com";
  beast::ssl_stream<beast::tcp_stream> stream(ioContext, sslContext);
  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
    beast::error_code ec{static_cast<int>(::ERR_get_error()),
                         net::error::get_ssl_category()};
    return {};
  }
  resolver.emplace(ioContext);
  auto const results = resolver->resolve(host, "443");
  resolver.reset();

  beast::get_lowest_layer(stream).connect(results);
  // Perform the SSL handshake
  stream.handshake(net::ssl::stream_base::client);

  auto const path =
      (isFutures ? "/fapi/v1/ticker/price" : "/api/v3/ticker/price");
  http::request<http::string_body> request{http::verb::get, path, 11};
  request.set(http::field::host, host);
  request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request.set(http::field::accept, "*/*");

  http::write(stream, request);

  // Declare a container to hold the response
  http::response<http::string_body> response;
  beast::flat_buffer readBuffer;

  // Receive the HTTP response
  http::read(stream, readBuffer, response);
  boost::system::error_code ec;
  stream.shutdown(ec);

  auto &body = response.body();
  rapidjson::Document doc;
  doc.Parse(body.c_str(), body.size());

  assert(doc.IsArray());
  std::vector<std::string> result;
  result.reserve(doc.Size());

  for (size_t i = 0; i < doc.Size(); ++i) {
    assert(doc[i].IsObject());
    auto const item = doc[i].GetObj();
    std::string const symbol = item.FindMember("symbol")->value.GetString();
    result.push_back(symbol);
  }
  return result;
}
} // namespace binance
