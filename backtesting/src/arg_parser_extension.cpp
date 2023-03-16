#include "arguments_parser.hpp"
#include "global_data.hpp"

#include <thread>

namespace backtesting {
#ifdef BT_USE_WITH_INDICATORS

  void scheduleCandlestickTask(unsigned long long, unsigned long long);
  void processDepthStream(trade_map_td &tradeMap,
                          std::vector<std::vector<std::string>> &&config);
#else
  void processDepthStream(trade_map_td &tradeMap);
#endif

void aggregateTradesImpl();
void liquidationOfPositionsImpl();
} // namespace backtesting

int backtesting_t::run() {
  if (!isReady())
    return EXIT_FAILURE;

  auto &globalRtData = global_data_t::instance();
  auto &csvFilenames = globalRtData.listOfFiles;

  // implemented in aggregate_trade.cpp
  std::thread{[] { backtesting::aggregateTradesImpl(); }}.detach();

  // implemented in signals.cpp
  std::thread{[] { backtesting::liquidationOfPositionsImpl(); }}.detach();

#ifdef BT_USE_WITH_INDICATORS
  std::thread{[] { backtesting::candlestickProcessingImpl(); }}.detach();
#endif
  // std::thread{[] { backtesting::bookTickerProcessingThreadImpl();
  // }}.detach();

  std::unique_ptr<std::thread> depthStreamThread = nullptr;
  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    depthStreamThread.reset(new std::thread{[&,csData = iter->second]() mutable {
#ifdef BT_USE_WITH_INDICATORS
      backtesting::processDepthStream(csData , std::move(globalRtData.indicatorConfig));
      backtesting::scheduleCandlestickTask(globalRtData.startTime, globalRtData.endTime);
#else
      backtesting::processDepthStream(csData);
#endif
    }});
  }

  if (depthStreamThread)
    depthStreamThread->join();
  return 0;
}
