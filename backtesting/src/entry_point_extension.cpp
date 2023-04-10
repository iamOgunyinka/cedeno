#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <boost/asio/io_context.hpp>

#include "entry_point.hpp"
#include "global_data.hpp"
#include "indicator_data.hpp"
#include "tick.hpp"

#include <thread>

namespace net = boost::asio;

namespace backtesting {
#ifdef BT_USE_WITH_INDICATORS
void scheduleCandlestickTask(unsigned long long, unsigned long long);
void processDepthStream(std::shared_ptr<net::io_context>,
                        trade_map_td &depthMap, trade_map_td &tradeMap,
                        std::vector<std::vector<std::string>> &&config);
#else
void processDepthStream(std::shared_ptr<net::io_context>,
                        trade_map_td &depthMap, trade_map_td &tradeMap);
#endif

void aggregateTradesImpl();
void liquidationOfPositionsImpl();
} // namespace backtesting

int backtesting_t::run() {
  if (!isReady())
    return EXIT_FAILURE;

  auto &globalRtData = global_data_t::instance();
  auto &csvFilenames = globalRtData.listOfFiles;

  if (globalRtData.onStart)
    globalRtData.onStart();

  // implemented in aggregate_trade.cpp
  std::thread{[] { backtesting::aggregateTradesImpl(); }}.detach();

  // implemented in signals.cpp
  std::thread{[] { backtesting::liquidationOfPositionsImpl(); }}.detach();

#ifdef BT_USE_WITH_INDICATORS
  std::thread{[] { backtesting::candlestickProcessingImpl(); }}.detach();
#endif

  std::unique_ptr<std::thread> depthStreamThread = nullptr;
  auto ioContext = backtesting::getContextObject();

  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    auto &depthData = iter->second;
    auto &tradeData = csvFilenames[TRADE];
    depthStreamThread = std::make_unique<std::thread>([&globalRtData,
                                                       &depthData, &tradeData,
                                                       ioContext]() mutable {
#ifdef BT_USE_WITH_INDICATORS
      backtesting::processDepthStream(ioContext, depthData, tradeData,
                                      std::move(globalRtData.indicatorConfig));
      backtesting::scheduleCandlestickTask(globalRtData.startTime,
                                           globalRtData.endTime);
#else
      backtesting::processDepthStream(ioContext, depthData, tradeData);
#endif
    });
  }

#ifdef BT_USE_WITH_INDICATORS
  auto tickInstance = backtesting::ticker_t::instance();
  tickInstance->addTicks(globalRtData.ticks);
  tickInstance->setCallback(globalRtData.onTick);
  tickInstance->startTimers();
#endif

  if (depthStreamThread)
    depthStreamThread->join();
  return 0;
}
