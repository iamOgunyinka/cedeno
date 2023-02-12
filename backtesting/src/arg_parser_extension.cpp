#include <boost/asio/io_context.hpp>

#include "arguments_parser.hpp"
#include "global_data.hpp"

#include <thread>

namespace net = boost::asio;

namespace backtesting {
void processDepthStream(net::io_context &, trade_map_td &tradeMap);

void processTickerStream(trade_map_td const &tradeMap) {
  // todo
}

void aggregateTradesImpl();
void candlestickProcessingImpl();
void bookTickerProcessingThreadImpl();
} // namespace backtesting

int backtesting_t::run() {
  if (!isReady())
    return EXIT_FAILURE;

  auto &globalRtData = global_data_t::instance();
  auto &csvFilenames = globalRtData.listOfFiles;

  std::thread{[] { backtesting::aggregateTradesImpl(); }}.detach();
  std::thread{[] { backtesting::candlestickProcessingImpl(); }}.detach();
  std::thread{[] { backtesting::bookTickerProcessingThreadImpl(); }}.detach();

  boost::asio::io_context ioContext;
  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    std::thread{[csData = iter->second, &ioContext]() mutable {
      backtesting::processDepthStream(ioContext, csData);
    }}.detach();
  }

  std::this_thread::sleep_for(std::chrono::seconds(10));
  ioContext.run();
  return 0;
}
