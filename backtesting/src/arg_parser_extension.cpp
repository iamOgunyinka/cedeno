#include "arguments_parser.hpp"
#include "global_data.hpp"

#include <thread>

namespace backtesting {
void processDepthStream(trade_map_td &tradeMap);
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

  std::unique_ptr<std::thread> depthStreamThread = nullptr;
  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    depthStreamThread.reset(new std::thread{[csData = iter->second]() mutable {
      backtesting::processDepthStream(csData);
    }});
  }

  if (depthStreamThread)
    depthStreamThread->join();
  return 0;
}
