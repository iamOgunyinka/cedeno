// backtesting.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include <CLI11/CLI11.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>

#include "candlestick_data.hpp"
#include "depth_data.hpp"

void processBookTickerStream(backtesting::trade_map_td const &tradeMap) {
  //
}

void processTickerStream(backtesting::trade_map_td const &tradeMap) {
  //
}

using namespace backtesting::utils;

int main(int argc, char **argv) {
  CLI::App app{"backtesting software for Creed & Bear LLC"};

  using backtesting::stringlist_t;

  stringlist_t streams;
  stringlist_t tradeTypes;
  stringlist_t tokenList;

  std::string rootDir;
  std::string dateFromStr;
  std::string dateToStr;

  app.add_option("--tokens", tokenList,
                 "a list of token pairs [e.g. btcusdt(default), ethusdt]");
  app.add_option("--streams", streams,
                 "A list of the streams(s) to run. Valid options are: "
                 "[trade, ticker, bookticker, depth(default), kline]");
  app.add_option("--trade-types", tradeTypes,
                 "A list of trade types. Valid options are: "
                 "[futures, spot(default)]");
  app.add_option("--start-date", dateFromStr,
                 "the start datetime (e.g. 2022-12-01 00:00:00)");
  app.add_option("--end-date", dateToStr,
                 "the end datetime (e.g. 2022-12-31 23:59:50)");
  app.add_option(
      "--root-dir", rootDir,
      "Root directory where the historical data are stored (default: `pwd`)");

  CLI11_PARSE(app, argc, argv);
#ifdef _DEBUG
  std::cout << "start date: " << dateFromStr << "\n"
            << "end date: " << dateToStr << "\n"
            << "rootDir: " << rootDir << "\n";

  for (auto const &token : tokenList)
    std::cout << "token: " << token << std::endl;
  for (auto const &s : streams)
    std::cout << "stream: " << s << std::endl;
  for (auto const &t : tradeTypes)
    std::cout << "trade: " << t << std::endl;

#endif // _DEBUG

  if (streams.empty()) {
    streams.push_back(DEPTH);
  } else {
    std::vector<std::string> const validStreams{TRADE, TICKER, BTICKER,
                                                CANDLESTICK, DEPTH};
    for (auto const &stream : streams) {
      if (!listContains(validStreams, stream)) {
        std::cerr << "'" << stream << "' is not a valid stream type";
        return EXIT_FAILURE;
      }
    }
  }

  if (tradeTypes.empty()) {
    tradeTypes.push_back(SPOT);
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    for (auto const &trade : tradeTypes) {
      if (!listContains(validTrades, trade)) {
        std::cerr << "'" << trade << "' is not a valid trade type";
        return EXIT_FAILURE;
      }
    }
  }

  if (tokenList.empty()) {
    tokenList.push_back("BTCUSDT");
#ifdef _DEBUG
    tokenList.push_back("ETHUSDT");
    tokenList.push_back("RUNEUSDT");
#endif // _DEBUG
  }

  if (rootDir.empty())
#ifdef _DEBUG
    rootDir = "C:\\Users\\Administrator\\Desktop\\example\\backtestingFiles";
#else
    rootDir = ".";
#endif // _DEBUG

  if (!std::filesystem::exists(rootDir)) {
    std::cerr << "'" << rootDir << "' does not exist" << std::endl;
    return EXIT_FAILURE;
  }

#ifdef _DEBUG
  if (dateFromStr.empty()) {
    constexpr std::size_t const last24hrs = 3'600 * 24;
    dateFromStr = backtesting::utils::currentTimeToString(
                      std::time(nullptr) - last24hrs, "-")
                      .value() +
                  " 00:00:00";
  }

  if (dateToStr.empty()) {
    dateToStr = backtesting::utils::currentTimeToString(std::time(nullptr), "-")
                    .value() +
                " 11:59:59";
  }
#endif // _DEBUG

  std::time_t startTime = 0, endTime = 0;
  if (auto const optStartTime = stringToTimeT(dateFromStr);
      optStartTime.has_value()) {
    startTime = *optStartTime;
  } else {
    std::cerr << "Unable to calculate the start date from user input"
              << std::endl;
    return EXIT_FAILURE;
  }

  if (auto const optEndTime = stringToTimeT(dateToStr);
      optEndTime.has_value()) {
    endTime = *optEndTime;
  } else {
    std::cerr << "Unable to calculate the end date from user input"
              << std::endl;
    return EXIT_FAILURE;
  }

  if (startTime > endTime)
    std::swap(startTime, endTime);

  filename_map_td const csvFiles = getListOfCSVFiles(
      tokenList, tradeTypes, streams, startTime, endTime, rootDir);
  if (csvFiles.empty()) {
    std::cerr << "No files found matching that criteria" << std::endl;
    return EXIT_FAILURE;
  }

  if (auto const iter = csvFiles.find(BTICKER); iter != csvFiles.cend()) {
    std::thread{[bookTickerInfo = iter->second] {
      processBookTickerStream(bookTickerInfo);
    }}.detach();
  }

  if (auto const iter = csvFiles.find(TICKER); iter != csvFiles.cend()) {
    std::thread{[tickerInfo = iter->second] {
      processTickerStream(tickerInfo);
    }}.detach();
  }

  if (auto const iter = csvFiles.find(CANDLESTICK); iter != csvFiles.cend()) {
    std::thread{[csData = iter->second] {
      backtesting::processCandlestickStream(csData);
    }}.detach();
  }

  net::io_context ioContext;
  if (auto iter = csvFiles.find(DEPTH); iter != csvFiles.end()) {
    std::thread{[csData = iter->second, &ioContext]() mutable {
      backtesting::processDepthStream(ioContext, csData);
    }}.detach();
  }

  std::this_thread::sleep_for(std::chrono::seconds(100));
  ioContext.run();
  return 0;
}
