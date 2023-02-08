#include "arguments_parser.hpp"
#include "global_data.hpp"
#include <boost/asio/io_context.hpp>
#include <thread>

namespace net = boost::asio;

namespace backtesting {
void processDepthStream(net::io_context &, trade_map_td &tradeMap);
void processBookTickerStream(trade_map_td const &tradeMap) {
  // todo
}

void processTickerStream(trade_map_td const &tradeMap) {
  // todo
}
} // namespace backtesting

int backtesting_t::run() {
  if (!isReady())
    return EXIT_FAILURE;

  auto &globalRtData = global_data_t::instance();
  auto &csvFilenames = globalRtData.listOfFiles;
  if (auto const iter = csvFilenames.find(BTICKER);
      iter != csvFilenames.cend()) {
    std::thread{[bookTickerInfo = iter->second] {
      backtesting::processBookTickerStream(bookTickerInfo);
    }}.detach();
  }

  if (auto const iter = csvFilenames.find(TICKER);
      iter != csvFilenames.cend()) {
    std::thread{[tickerInfo = iter->second] {
      backtesting::processTickerStream(tickerInfo);
    }}.detach();
  }
  /*
  if (auto const iter = csvFilenames.find(CANDLESTICK);
      iter != csvFilenames.cend()) {
    std::thread{[csData = iter->second] {
      backtesting::processCandlestickStream(csData);
    }}.detach();
  }
  */

  boost::asio::io_context ioContext;
  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    std::thread{[csData = iter->second, &ioContext]() mutable {
      backtesting::processDepthStream(ioContext, csData);
    }}.detach();
  }

  std::this_thread::sleep_for(std::chrono::seconds(20));
  ioContext.run();
  return 0;
}
