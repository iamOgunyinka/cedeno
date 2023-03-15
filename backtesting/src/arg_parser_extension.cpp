#include "arguments_parser.hpp"
#include "global_data.hpp"

#include <thread>

namespace backtesting {
void processDepthStream(trade_map_td &tradeMap
#ifdef BT_USE_WITH_INDICATORS
                        , std::vector<std::vector<std::string>> &&config
#endif
                        );
void aggregateTradesImpl();
void candlestickProcessingImpl();
void bookTickerProcessingThreadImpl();
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

  // std::thread{[] { backtesting::candlestickProcessingImpl(); }}.detach();
  // std::thread{[] { backtesting::bookTickerProcessingThreadImpl();
  // }}.detach();

  std::unique_ptr<std::thread> depthStreamThread = nullptr;
  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    depthStreamThread.reset(new std::thread{[&,csData = iter->second]() mutable {
      backtesting::processDepthStream(csData
#ifdef BT_USE_WITH_INDICATORS
        , std::move(globalRtData.indicatorConfig)
#endif
      );
    }});
  }

  if (depthStreamThread)
    depthStreamThread->join();
  return 0;
}
