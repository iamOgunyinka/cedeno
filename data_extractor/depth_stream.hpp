#pragma once

#include "container.hpp"
#include "websocket_stream_base.hpp"

namespace binance {

struct depth_data_t {
  std::string tokenName;
  time_t eventTime = 0;
  time_t transactionTime = 0;
  uint64_t firstUpdateID = 0;
  uint64_t finalUpdateID = 0;
  uint64_t finalStreamUpdateID = 0;

  struct depth_meta_t {
    double priceLevel = 0.0;
    double quantity = 0.0;
  };
  std::vector<depth_meta_t> bids;
  std::vector<depth_meta_t> asks;
};

class depth_stream_t : public websocket_stream_base_t {

public:
  depth_stream_t(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_type_e const tradeType, token_filename_map_t &tradeMap)
      : websocket_stream_base_t(ioContext, sslContext, tradeType),
        m_tradeMap(tradeMap), m_tradeType(tradeType) {
    for (auto const &[key, _] : tradeMap.dataMap)
      makeSubscriptionFor(key);
    startMonitoringThread();
  }
  ~depth_stream_t() {}

private:
  void startMonitoringThread();
  void startMonitoringThreadImpl();
  void writeCSVHeader(binance::locked_file_t &);
  void writeCSVBody(depth_data_t &&);
  std::string getSubscriptionMessage(std::string const &token) const override;
  std::string getStreamType() const override { return "depth"; }
  void processResponse(char const *const str, size_t const length) override;
  void getDepthSnapshotNoAsync();

  utils::waitable_container_t<depth_data_t> m_dataList;
  token_filename_map_t &m_tradeMap;
  trade_type_e const m_tradeType;
  bool m_isFirstRequest = true;
};

struct temp_token_data_t {
  std::string fullTokenName;
  std::string base;
  std::string quote;
};

std::vector<temp_token_data_t>
fetchToken(net::io_context &, net::ssl::context &, trade_type_e const);

} // namespace binance
