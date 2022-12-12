#include "candlestick_futures_stream.hpp"
#include "candlestick_spot_stream.hpp"
#include "ticker_stream.hpp"
#include "trade_stream.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <filesystem>
#include <map>

#define CANDLESTICK "kline"
#define BTICKER "bookTicker"
#define TICKER "ticker"
#define TRADE "trade"
#define ORDERBOOK "orderBook"
#define SPOT "spot"
#define FUTURES "futures"

namespace net = boost::asio;

using stream_type_td = std::string;
using trade_type_td = std::string;
using trade_map_td = std::map<trade_type_td, binance::token_filename_map_t>;
using filename_map_td = std::map<stream_type_td, trade_map_td>;

using binance::trade_type_e;

bool createAllFiles(filename_map_td &filenameMap,
                    std::vector<std::string> const &tokens) {
  auto const currentDate =
      binance::currentTimeToString(binance::time_type_e::date);
  auto const currentTime =
      binance::currentTimeToString(binance::time_type_e::time);
  if (!(currentDate && currentTime)) {
    std::cerr << "Unable to get the current local date/time" << std::endl;
    return false;
  }

  std::filesystem::path const rootPath = std::filesystem::current_path();
  for (auto const &tokenName_ : tokens) {
    auto const tokenName = binance::toUpperString(tokenName_);
    for (auto const &streamType :
         {CANDLESTICK, BTICKER, TICKER, TRADE, ORDERBOOK}) {
      for (auto const &tradeType : {SPOT, FUTURES}) {
        auto const fullPath =
            rootPath / tokenName / *currentDate / streamType / tradeType;
        if (!std::filesystem::exists(fullPath)) {
          std::filesystem::create_directories(fullPath);
        }
        auto const filePath = (fullPath / (*currentTime + ".csv")).string();
        auto &dataMap = filenameMap[streamType][tradeType].dataMap;
        if (dataMap.find(filePath) == dataMap.end()) {
          dataMap[tokenName] = binance::locked_file_t(filePath);
        } else {
          dataMap[tokenName].changeFilename(filePath);
        }
        dataMap[tokenName].rewriteHeader(true);
      }
    }
  }
  return true;
}

void fetchBookTicker(net::io_context &ioContext, net::ssl::context &sslContext,
                     trade_map_td &tradeMap) {
  binance::book_ticker_stream_t fTickerStream(
      ioContext, sslContext, trade_type_e::futures, tradeMap[FUTURES]);

  binance::book_ticker_stream_t sTickerStream(
      ioContext, sslContext, trade_type_e::spot, tradeMap[SPOT]);

  sTickerStream.start();
  fTickerStream.start();
  ioContext.run();
}

void fetchTicker(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_map_td &tradeMap) {
  binance::ticker_stream_t futuresStream(
      ioContext, sslContext, trade_type_e::futures, tradeMap[FUTURES]);
  binance::ticker_stream_t spotStream(ioContext, sslContext, trade_type_e::spot,
                                      tradeMap[SPOT]);
  futuresStream.start();
  spotStream.start();
  ioContext.run();
}

void fetchTradeStream(net::io_context &ioContext, net::ssl::context &sslContext,
                      trade_map_td &tradeMap) {
  binance::trade_stream_t fTradeStream(
      ioContext, sslContext, trade_type_e::futures, tradeMap[FUTURES]);
  binance::trade_stream_t sTradeStream(ioContext, sslContext,
                                       trade_type_e::spot, tradeMap[SPOT]);

  fTradeStream.start();
  sTradeStream.start();
  ioContext.run();
}

void fetchCandlestick(net::io_context &ioContext, net::ssl::context &sslContext,
                      trade_map_td &tradeMap) {
  binance::candlestick_futures_stream_t fTradeStream(ioContext, sslContext,
                                                     tradeMap[FUTURES]);
  binance::candlestick_spot_stream_t sTradeStream(ioContext, sslContext,
                                                  tradeMap[SPOT]);
  fTradeStream.start();
  sTradeStream.start();
  ioContext.run();
}

tm getCurrentDate() {
  time_t const t = std::time(nullptr);
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif // _MSC_VER

  return *std::localtime(&t);
}

void timeWatcher(filename_map_td &filenameMap,
                 std::vector<std::string> const &tokens) {
  tm initialDate = getCurrentDate();
  while (true) {
    auto const currentDate = getCurrentDate();
    if (currentDate.tm_hour != initialDate.tm_hour) {
      createAllFiles(filenameMap, tokens);
      std::cout << "The time has changed from " << initialDate.tm_hour << ":"
                << initialDate.tm_min << " to " << currentDate.tm_hour << ":"
                << currentDate.tm_min << std::endl;

      initialDate = currentDate;
      // sleep for 58 minutes
      std::this_thread::sleep_for(std::chrono::minutes(58));
      continue;
    }
    std::this_thread::sleep_for(std::chrono::seconds(60));
  }
}

int main(int const argc, char const **argv) {
  // auto const maxThreadSize = std::thread::hardware_concurrency();
  net::io_context ioContext;
  auto sslContext =
      std::make_unique<net::ssl::context>(net::ssl::context::tlsv12_client);
  sslContext->set_default_verify_paths();
  sslContext->set_verify_mode(boost::asio::ssl::verify_none);

  std::vector<std::string> const tokens{"BNBUSDT", "BTCUSDT", "RUNEUSDT",
                                        "ETHUSDT"};
  filename_map_td filenameMap;
  if (!createAllFiles(filenameMap, tokens))
    return -1;

  std::cout << "Starting the trade stream..." << std::endl;
  std::thread([&] {
    fetchTradeStream(ioContext, *sslContext, filenameMap[TRADE]);
  }).detach();

  std::cout << "Starting the ticker stream..." << std::endl;
  std::thread([&] {
    fetchTicker(ioContext, *sslContext, filenameMap[TICKER]);
  }).detach();

  std::cout << "Starting the bookTicker stream..." << std::endl;
  std::thread([&] {
    fetchBookTicker(ioContext, *sslContext, filenameMap[BTICKER]);
  }).detach();

  std::cout << "Starting the candleStick stream..." << std::endl;
  std::thread([&] {
    fetchCandlestick(ioContext, *sslContext, filenameMap[CANDLESTICK]);
  }).detach();

  std::cout << "Starting the periodic time watcher..." << std::endl;
  std::thread([&] { timeWatcher(filenameMap, tokens); }).detach();

  std::cout << "Program started successfully!" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  ioContext.run();
  return 0;
}
