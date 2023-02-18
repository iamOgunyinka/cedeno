#include "data_extractor.hpp"

#include "candlestick_futures_stream.hpp"
#include "candlestick_spot_stream.hpp"
#include "depth_stream.hpp"
#include "ticker_stream.hpp"
#include "trade_stream.hpp"
#include <CLI11/CLI11.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <filesystem>
#include <map>
#include <spdlog/spdlog.h>

#define CANDLESTICK "kline"
#define BTICKER "bookticker"
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
                    std::vector<std::string> const &tokens,
                    std::vector<std::string> const &streams,
                    std::vector<std::string> const &tradeTypes) {
  auto const currentDate = currentTimeToString(time_type_e::date);
  auto const currentTime = currentTimeToString(time_type_e::time);
  if (!(currentDate && currentTime)) {
    spdlog::error("Unable to get the current local date/time");
    return false;
  }

  std::filesystem::path const rootPath =
      std::filesystem::current_path() / "backtestingFiles";
  for (auto const &tokenName_ : tokens) {
    auto const tokenName = toUpperString(tokenName_);
    for (auto const &streamType : streams) {
      for (auto const &tradeType : tradeTypes) {
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

  spdlog::info("Starting the depth stream...");
  sDepthStream.start();
  fDepthStream.start();
  ioContext.run();
  spdlog::info("[Done] running the depth streams...");
}

void fetchBookTicker(net::io_context &ioContext, net::ssl::context &sslContext,
                     trade_map_td &tradeMap) {
  book_ticker_stream_t fTickerStream(ioContext, sslContext,
                                     trade_type_e::futures, tradeMap[FUTURES]);

  book_ticker_stream_t sTickerStream(ioContext, sslContext, trade_type_e::spot,
                                     tradeMap[SPOT]);

  spdlog::info("Starting the bookTicker stream...");
  sTickerStream.start();
  fTickerStream.start();
  ioContext.run();
  spdlog::info("[Done] running the bookTicker streams...");
}

void fetchTicker(net::io_context &ioContext, net::ssl::context &sslContext,
                 trade_map_td &tradeMap) {
  ticker_stream_t futuresStream(ioContext, sslContext, trade_type_e::futures,
                                tradeMap[FUTURES]);
  ticker_stream_t spotStream(ioContext, sslContext, trade_type_e::spot,
                             tradeMap[SPOT]);

  spdlog::info("Starting the ticker stream...");
  futuresStream.start();
  spotStream.start();
  ioContext.run();
  spdlog::info("[Done] running the ticker streams...");
}

void fetchTradeStream(net::io_context &ioContext, net::ssl::context &sslContext,
                      trade_map_td &tradeMap) {
  trade_stream_t fTradeStream(ioContext, sslContext, trade_type_e::futures,
                              tradeMap[FUTURES]);
  trade_stream_t sTradeStream(ioContext, sslContext, trade_type_e::spot,
                              tradeMap[SPOT]);

  spdlog::info("Starting the trade stream...");
  fTradeStream.start();
  sTradeStream.start();
  ioContext.run();
  spdlog::info("[Done] running the trade streams...");
}

void fetchCandlestick(net::io_context &ioContext, net::ssl::context &sslContext,
                      trade_map_td &tradeMap) {
  candlestick_futures_stream_t fTradeStream(ioContext, sslContext,
                                            tradeMap[FUTURES]);
  candlestick_spot_stream_t sTradeStream(ioContext, sslContext, tradeMap[SPOT]);
  spdlog::info("Starting the kline stream...");
  fTradeStream.start();
  sTradeStream.start();
  ioContext.run();
  spdlog::info("[Done] running the kline streams...");
}

tm getCurrentDate() {
  time_t const t = std::time(nullptr);
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif // _MSC_VER

  return *std::localtime(&t);
}

void timeWatcher(filename_map_td &filenameMap,
                 std::vector<std::string> const &tokens,
                 std::vector<std::string> const &streams,
                 std::vector<std::string> const &tradeTypes) {
  spdlog::info("Starting the periodic time watcher...");

  tm initialDate = getCurrentDate();
  while (true) {
    auto const currentDate = getCurrentDate();
    if (currentDate.tm_hour != initialDate.tm_hour) {
      createAllFiles(filenameMap, tokens, streams, tradeTypes);
      spdlog::info("The time has changed from {}:{} to {}:{}",
                   initialDate.tm_hour, initialDate.tm_min, currentDate.tm_hour,
                   currentDate.tm_min);

      initialDate = currentDate;
      // sleep for 58 minutes
      std::this_thread::sleep_for(std::chrono::minutes(58));
      continue;
    }
    std::this_thread::sleep_for(std::chrono::seconds(60));
  }
}

void fetchAndSaveAllTokens(net::io_context &ioContext,
                           net::ssl::context &sslContext) {
  spdlog::info("Fetching all tokens...");
  std::vector<std::pair<trade_type_e, char const *>> const pair{
      {trade_type_e::futures, "futures.csv"}, {trade_type_e::spot, "spot.csv"}};

  auto const dirPath = std::filesystem::current_path() / "backtestingFiles";
  if (!std::filesystem::exists(dirPath))
    std::filesystem::create_directories(dirPath);

  for (auto const &[tradeType, filename] : pair) {
    auto tokenList = fetchToken(ioContext, sslContext, tradeType);
    if (tokenList.empty())
      continue;

    auto const path = dirPath / filename;
    if (std::filesystem::exists(path))
      std::filesystem::remove(path);

    std::ofstream file(path, std::ios::trunc);
    if (!file)
      continue;
    for (auto const &token : tokenList) {
      file << fmt::format("{},{},{}\n", token.fullTokenName, token.base,
                          token.quote);
    }
    file.close();
  }
  spdlog::info("[DONE] Fetching all tokens...");
}
} // namespace binance

bool isCaseInsensitiveStringCompare(std::string const &s,
                                    std::string const &t) {
  auto const len = s.length();
  if (len != t.length())
    return false;
  for (std::string::size_type i = 0; i < len; ++i)
    if (tolower(t[i]) != tolower(s[i]))
      return false;
  return true;
}

bool listContains(std::vector<std::string> const &container,
                  std::string const &t) {
  for (auto const &data : container)
    if (isCaseInsensitiveStringCompare(data, t))
      return true;
  return false;
}

data_extractor_t::data_extractor_t()
    : m_ioContext(new net::io_context(std::thread::hardware_concurrency())) {}

data_extractor_t::~data_extractor_t() {
  stop();
  delete m_ioContext;
  m_ioContext = nullptr;
}

int data_extractor_t::run(size_t const argc, char **argv) {
  CLI::App app{"C&B token data extractor"};

  std::vector<std::string> tokenList, trades, streams;
  int timerInSeconds = 300; // 5 minutes

  app.add_option("--tokens", tokenList,
                 "a list of token pairs [e.g. btcusdt(default), ethusdt]");
  app.add_option("-r,--run-for", timerInSeconds,
                 "run the data extractor for N seconds (default: 5 minutes)");
  app.add_option("--trade-type", trades,
                 "trade types (e.g. spot(default), futures)");
  app.add_option(
      "--streams", streams,
      "streams (e.g. depth(default), bookTicker, ticker, kline, trade");
  try {
    app.parse(argc, argv);
  } catch (CLI::ParseError &e) {
    if (e.get_exit_code() != 0)
      spdlog::error(e.what());
    app.exit(e);
    return false;
  }

  auto &ioContext = *m_ioContext;
  ioContext.reset();

  auto sslContext =
      std::make_unique<net::ssl::context>(net::ssl::context::tlsv12_client);
  sslContext->set_default_verify_paths();
  sslContext->set_verify_mode(boost::asio::ssl::verify_none);

  binance::fetchAndSaveAllTokens(ioContext, *sslContext);
  if (tokenList.empty()) {
    tokenList.push_back("BTCUSDT");
    spdlog::info("No tokens supplied, will list only for '{}'", tokenList[0]);
  } else {
    spdlog::info("Will be gathering information for {} tokens",
                 tokenList.size());
  }

  if (streams.empty()) {
    spdlog::info("Streams not specified, will use 'DEPTH' as default");
    streams.push_back(DEPTH);
  } else {
    std::vector<std::string> const validStreams{INDC_TRADE, TICKER, BTICKER,
                                                CANDLESTICK, DEPTH};
    for (auto &stream : streams) {
      if (!listContains(validStreams, stream)) {
        spdlog::error("'{}' is not a valid stream type", stream);
        return -1;
      }
      for (auto &s : stream)
        s = tolower(s);
    }
  }

  if (trades.empty()) {
    spdlog::info("trade type not specified, will use 'SPOT' as default");
    trades.push_back(SPOT);
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    for (auto &trade : trades) {
      if (!listContains(validTrades, trade)) {
        spdlog::error("'{}' is not a valid trade type", trade);
        return -1;
      }
      for (auto &t : trade)
        t = tolower(t);
    }
  }

  binance::filename_map_td filenameMap;
  if (!binance::createAllFiles(filenameMap, tokenList, streams, trades))
    return -1;

  if (listContains(streams, INDC_TRADE)) {
    std::thread([&] {
      binance::fetchTradeStream(ioContext, *sslContext, filenameMap[INDC_TRADE]);
    }).detach();
  }

  if (listContains(streams, TICKER)) {
    std::thread([&] {
      binance::fetchTicker(ioContext, *sslContext, filenameMap[TICKER]);
    }).detach();
  }

  if (listContains(streams, BTICKER)) {
    std::thread([&] {
      binance::fetchBookTicker(ioContext, *sslContext, filenameMap[BTICKER]);
    }).detach();
  }

  if (listContains(streams, CANDLESTICK)) {
    std::thread([&] {
      binance::fetchCandlestick(ioContext, *sslContext,
                                filenameMap[CANDLESTICK]);
    }).detach();
  }

  if (listContains(streams, DEPTH)) {
    std::thread([&] {
      binance::fetchTokenDepth(ioContext, *sslContext, filenameMap[DEPTH]);
    }).detach();
  }

  std::thread([&] {
    binance::timeWatcher(filenameMap, tokenList, streams, trades);
  }).detach();

  spdlog::info("Program started successfully!");
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::optional<net::deadline_timer> timer = std::nullopt;

  if (timerInSeconds > 0) {
    spdlog::info("Stopping the program in '{}' seconds", timerInSeconds);
    if (timerInSeconds > 0) {
      timer.emplace(ioContext);
      timer->expires_from_now(boost::posix_time::seconds(timerInSeconds));
      timer->async_wait(
          [&ioContext](boost::system::error_code const &) mutable {
            spdlog::info("Stopping the program right now");
            ioContext.stop();
          });
    }
  }

  m_contextIsRunning = true;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  ioContext.run();
  // wait for all objects to be destroyed.
  std::this_thread::sleep_for(std::chrono::seconds(5));
  m_contextIsRunning = false;
  return 0;
}

bool data_extractor_t::stop() {
  if (m_contextIsRunning) {
    m_ioContext->stop();
    while (!m_contextIsRunning)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return true;
}
