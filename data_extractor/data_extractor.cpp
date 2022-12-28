#include "data_extractor.hpp"

#include "candlestick_futures_stream.hpp"
#include "candlestick_spot_stream.hpp"
#include "depth_stream.hpp"
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
#define DEPTH "depth"
#define SPOT "spot"
#define FUTURES "futures"

namespace net = boost::asio;

namespace binance {
using stream_type_td = std::string;
using trade_type_td = std::string;
using trade_map_td = std::map<trade_type_td, token_filename_map_t>;
using filename_map_td = std::map<stream_type_td, trade_map_td>;

bool createAllFiles(filename_map_td &filenameMap,
                    std::vector<std::string> const &tokens) {
  auto const currentDate = currentTimeToString(time_type_e::date);
  auto const currentTime = currentTimeToString(time_type_e::time);
  if (!(currentDate && currentTime)) {
    std::cerr << "Unable to get the current local date/time" << std::endl;
    return false;
  }

  std::filesystem::path const rootPath =
      std::filesystem::current_path() / "backtestingFiles";
  for (auto const &tokenName_ : tokens) {
    auto const tokenName = toUpperString(tokenName_);
    for (auto const &streamType :
         {CANDLESTICK, BTICKER, TICKER, TRADE, DEPTH}) {
      for (auto const &tradeType : {SPOT, FUTURES}) {
        auto const fullPath =
            rootPath / tokenName / *currentDate / streamType / tradeType;
        if (!std::filesystem::exists(fullPath)) {
          std::filesystem::create_directories(fullPath);
        }
        auto const filePath = (fullPath / (*currentTime + ".csv")).string();
        auto &dataMap = filenameMap[streamType][tradeType].dataMap;
        if (dataMap.find(filePath) == dataMap.end()) {
          dataMap[tokenName] = locked_file_t(filePath);
        } else {
          dataMap[tokenName].changeFilename(filePath);
        }
        dataMap[tokenName].rewriteHeader(true);
      }
    }
  }
  return true;
}

void fetchTokenDepth(net::io_context &ioContext, net::ssl::context &sslContext,
                     trade_map_td &tradeMap) {
  depth_stream_t fDepthStream(ioContext, sslContext, trade_type_e::futures,
                              tradeMap[FUTURES]);

  depth_stream_t sDepthStream(ioContext, sslContext, trade_type_e::spot,
                              tradeMap[SPOT]);

  sDepthStream.start();
  fDepthStream.start();
  ioContext.run();
}

void fetchBookTicker(net::io_context &ioContext, net::ssl::context &sslContext,
                     trade_map_td &tradeMap) {
  book_ticker_stream_t fTickerStream(ioContext, sslContext,
                                     trade_type_e::futures, tradeMap[FUTURES]);

  book_ticker_stream_t sTickerStream(ioContext, sslContext, trade_type_e::spot,
                                     tradeMap[SPOT]);

  sTickerStream.start();
  fTickerStream.start();
  ioContext.run();
}

void fetchTicker(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_map_td &tradeMap) {
  ticker_stream_t futuresStream(ioContext, sslContext, trade_type_e::futures,
                                tradeMap[FUTURES]);
  ticker_stream_t spotStream(ioContext, sslContext, trade_type_e::spot,
                             tradeMap[SPOT]);
  futuresStream.start();
  spotStream.start();
  ioContext.run();
}

void fetchTradeStream(net::io_context &ioContext, net::ssl::context &sslContext,
                      trade_map_td &tradeMap) {
  trade_stream_t fTradeStream(ioContext, sslContext, trade_type_e::futures,
                              tradeMap[FUTURES]);
  trade_stream_t sTradeStream(ioContext, sslContext, trade_type_e::spot,
                              tradeMap[SPOT]);

  fTradeStream.start();
  sTradeStream.start();
  ioContext.run();
}

void fetchCandlestick(net::io_context &ioContext, net::ssl::context &sslContext,
                      trade_map_td &tradeMap) {
  candlestick_futures_stream_t fTradeStream(ioContext, sslContext,
                                            tradeMap[FUTURES]);
  candlestick_spot_stream_t sTradeStream(ioContext, sslContext, tradeMap[SPOT]);
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

void fetchAllTokens(net::io_context &ioContext, net::ssl::context &sslContext) {
  std::vector<std::pair<trade_type_e, char const *>> const pair{
      {trade_type_e::futures, "futures.csv"}, {trade_type_e::spot, "spot.csv"}};

  auto const dirPath = std::filesystem::current_path() / "backtestingFiles";
  if (!std::filesystem::exists(dirPath))
    std::filesystem::create_directories(dirPath);

  for (auto const &[tradeType, filename] : pair) {
    auto const tokenList = fetchToken(ioContext, sslContext, tradeType);
    if (tokenList.empty())
      continue;

    auto const path = dirPath / filename;
    if (std::filesystem::exists(path))
      std::filesystem::remove(path);

    std::ofstream file(path, std::ios::trunc);
    if (!file)
      continue;
    for (auto const &token : tokenList)
      file << token << std::endl;
    file.close();
  }
}
} // namespace binance

data_extractor_t::data_extractor_t()
    : m_ioContext(new net::io_context(std::thread::hardware_concurrency())) {}

data_extractor_t::~data_extractor_t() {
  stop();
  delete m_ioContext;
  m_ioContext = nullptr;
}

int data_extractor_t::run(std::vector<std::string> &&args) {
  size_t const argc = args.size();

  auto &ioContext = *m_ioContext;
  ioContext.reset();

  auto sslContext =
      std::make_unique<net::ssl::context>(net::ssl::context::tlsv12_client);
  sslContext->set_default_verify_paths();
  sslContext->set_verify_mode(boost::asio::ssl::verify_none);

  std::vector<std::string> const tokens{"BNBUSDT", "BTCUSDT", "RUNEUSDT",
                                        "ETHUSDT"};
  std::cout << "Fetching all tokens..." << std::endl;
  binance::fetchAllTokens(ioContext, *sslContext);
  std::cout << "[DONE] Fetching all tokens..." << std::endl;

  binance::filename_map_td filenameMap;
  if (!binance::createAllFiles(filenameMap, tokens))
    return -1;

  std::cout << "Starting the trade stream..." << std::endl;
  std::thread([&] {
    binance::fetchTradeStream(ioContext, *sslContext, filenameMap[TRADE]);
  }).detach();

  std::cout << "Starting the ticker stream..." << std::endl;
  std::thread([&] {
    binance::fetchTicker(ioContext, *sslContext, filenameMap[TICKER]);
  }).detach();

  std::cout << "Starting the bookTicker stream..." << std::endl;
  std::thread([&] {
    binance::fetchBookTicker(ioContext, *sslContext, filenameMap[BTICKER]);
  }).detach();

  std::cout << "Starting the candleStick stream..." << std::endl;
  std::thread([&] {
    binance::fetchCandlestick(ioContext, *sslContext, filenameMap[CANDLESTICK]);
  }).detach();

  std::cout << "Starting the depth stream ..." << std::endl;
  std::thread([&] {
    binance::fetchTokenDepth(ioContext, *sslContext, filenameMap[DEPTH]);
  }).detach();

  std::cout << "Starting the periodic time watcher..." << std::endl;
  std::thread([&] { binance::timeWatcher(filenameMap, tokens); }).detach();

  std::cout << "Program started successfully!" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::optional<net::deadline_timer> timer = std::nullopt;

  if (argc == 2) {
    uint32_t const timerInSeconds = std::stoul(args[1]);
    std::cout << "Stopping the program in '" << timerInSeconds << "' seconds";
    if (timerInSeconds > 0) {
      timer.emplace(ioContext);
      timer->expires_from_now(boost::posix_time::seconds(timerInSeconds));
      timer->async_wait(
          [&ioContext](boost::system::error_code const &) mutable {
            std::cout << "Stopping the program right now" << std::endl;
            ioContext.stop();
          });
    }
  }

  ioContext.run();
  return 0;
}

bool data_extractor_t::stop() {
  auto &ioContext = *m_ioContext;
  if (!ioContext.stopped()) {
    ioContext.stop();
    while (!ioContext.stopped())
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return true;
}
