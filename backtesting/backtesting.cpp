// backtesting.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include "csv.hpp"
#include <CLI11/CLI11.hpp>
#include <ctime>
#include <iostream>
#include <optional>

#define CANDLESTICK "kline"
#define BTICKER "bookTicker"
#define TICKER "ticker"
#define TRADE "trade"
#define DEPTH "depth"

#define SPOT "spot"
#define FUTURES "futures"

using stringlist_t = std::vector<std::string>;
using stream_type_td = std::string;
using trade_type_td = std::string;
using fs_list_t = std::vector<std::filesystem::path>;
using trade_map_td = std::map<trade_type_td, fs_list_t>;
using filename_map_td = std::map<stream_type_td, trade_map_td>;

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

std::optional<std::time_t> stringToTimeT(std::string const &s) {
  std::tm tm;
  std::istringstream ss(s);
  ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
  if (ss.fail())
    return std::nullopt;
  std::time_t const time = mktime(&tm);
  if (time == -1)
    return std::nullopt;
  return time;
}

std::string toUpperString(std::string const &s) {
  std::string temp;
  temp.reserve(s.size());
  for (auto c : s)
    temp.push_back(std::toupper(c));
  return temp;
}

std::optional<std::string> currentTimeToString(std::time_t const currentTime,
                                               std::string const &delim) {
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning(disable : 4996)
#endif

  try {
    auto const tm_t = std::localtime(&currentTime);
    if (!tm_t) {
      return std::nullopt;
    }
    std::string output((std::size_t)32, (char)'\0');
    auto const format = "%Y" + delim + "%m" + delim + "%d";
    auto const stringLength =
        std::strftime(output.data(), output.size(), format.c_str(), tm_t);
    if (stringLength) {
      output.resize(stringLength);
      return output;
    }
  } catch (std::exception const &) {
  }
  return std::nullopt;
}

std::vector<std::time_t> intervalsBetweenDates(std::time_t start,
                                               std::time_t end) {
  std::time_t const endOfStartDate =
      stringToTimeT(currentTimeToString(start, "-").value() + " 23:59:59")
          .value();
  constexpr size_t const secondsInOneDay = 3'600 * 24;
  std::vector<std::time_t> timeList{start};
  if (endOfStartDate > end) // same day
    return timeList;
  start = endOfStartDate + 1;
  while (start <= end) {
    timeList.push_back(start);
    start += secondsInOneDay;
  }

  if (timeList.back() < end)
    timeList.push_back(end);
  return timeList;
}

filename_map_td
getListOfCSVFiles(stringlist_t const &tradeTypes, stringlist_t const &streams,
                  std::time_t const startTime, std::time_t const endTime,
                  std::string const &rootDir, std::string const &token) {
  std::filesystem::path const rootPath(rootDir);
  auto const intervals = intervalsBetweenDates(startTime, endTime);
  auto const tokenName = toUpperString(token);

  filename_map_td paths;
  for (auto const &interval : intervals) {
    auto const date = currentTimeToString(interval, "_").value();
    for (auto const &streamType : streams) {
      for (auto const &tradeType : tradeTypes) {
        auto const fullPath =
            rootPath / tokenName / date / streamType / tradeType;
        if (!std::filesystem::exists(fullPath))
          continue;
        for (auto const &file :
             std::filesystem::recursive_directory_iterator(fullPath)) {
          if (std::filesystem::is_regular_file(file))
            paths[streamType][tradeType].push_back(file);
        }
      }
    }
  }
  return paths;
}

std::vector<std::string> split_string(std::string const &str,
                                      char const *delim) {
  std::size_t const delimLength = std::strlen(delim);
  std::size_t fromPos{};
  std::size_t index{str.find(delim, fromPos)};
  if (index == std::string::npos) {
    return {str};
  }
  std::vector<std::string> result{};
  while (index != std::string::npos) {
    result.emplace_back(str.data() + fromPos, index - fromPos);
    fromPos = index + delimLength;
    index = str.find(delim, fromPos);
  }

  if (fromPos < str.length()) {
    result.emplace_back(str.data() + fromPos, str.size() - fromPos);
  }
  return result;
}

std::chrono::seconds stringToStdInterval(std::string &str) {
  char const ch = str.back();
  str.pop_back();

  std::size_t const value = std::stoul(str);
  switch (ch) {
  case 's': // seconds
    return std::chrono::seconds(value);
  case 'm': // minutes
    return std::chrono::seconds(value * 60);
  case 'h': // hour
    return std::chrono::seconds(value * 3'600);
  case 'd': // days
    return std::chrono::seconds(value * 3'600 * 24);
  case 'w':
    return std::chrono::seconds(value * 3'600 * 24 * 7);
  case 'M':
    return std::chrono::seconds(value * 3'600 * 24 * 7 * 12);
  }
  return std::chrono::seconds(0);
}

struct candlestick_data_t {
  std::chrono::seconds intervalInSeconds;
  uint64_t eventTime;
  uint64_t startTime;
  uint64_t closeTime;
  uint64_t firstTradeID;
  uint64_t lastTradeID;
  double openPrice;
  double closePrice;
  double highPrice;
  double lowPrice;
  double baseAssetVolume;
  double quoteAssetVolume;
  double tbBaseAssetVolume;  // Taker buy base asset volume
  double tbQuoteAssetVolume; // Taker buy quote asset volume
  size_t numberOfTrades;
  size_t klineIsClosed;

  static candlestick_data_t dataFromCSVLine(std::string const &str) {
    auto splittedString = split_string(str, ",");
    candlestick_data_t result;

    if (splittedString.size() != 16) {
      result.eventTime = 0;
      return result;
    }
    result.eventTime = std::stoull(splittedString[0]);
    result.startTime = std::stoull(splittedString[1]);
    result.closeTime = std::stoull(splittedString[2]);
    result.intervalInSeconds = stringToStdInterval(splittedString[3]);
    result.firstTradeID = std::stoull(splittedString[4]);
    result.lastTradeID = std::stoull(splittedString[5]);
    result.openPrice = std::stod(splittedString[6]);
    result.closePrice = std::stod(splittedString[7]);
    result.highPrice = std::stod(splittedString[8]);
    result.lowPrice = std::stod(splittedString[9]);
    result.baseAssetVolume = std::stod(splittedString[10]);
    result.numberOfTrades = std::stoul(splittedString[11]);
    result.klineIsClosed = std::stoul(splittedString[12]);
    result.quoteAssetVolume = std::stod(splittedString[13]);
    result.tbBaseAssetVolume = std::stod(splittedString[14]);
    result.tbQuoteAssetVolume = std::stod(splittedString[15]);
    return result;
  }
};

struct depth_data_t {
  std::string tokenName;
  time_t eventTime = 0;
  time_t transactionTime = 0;
  uint64_t firstUpdateID = 0;
  uint64_t finalUpdateID = 0;
  uint64_t finalStreamUpdateID = 0;

  struct depth_meta_t {
    double priceLevel = 0.0;
    double quantity = 0.0;
  };
  std::vector<depth_meta_t> bids;
  std::vector<depth_meta_t> asks;
};

template <typename T> struct data_streamer_t {
  data_streamer_t(fs_list_t const &fileNames) {
    m_paths.reserve(fileNames.size());
    for (auto const &filename : fileNames)
      m_paths.push_back({filename.string(), false});
  }

private:
  struct internal_data_t {
    std::string filename;
    bool isOpened;
  };
  std::vector<internal_data_t> m_paths;
};

void processBookTickerStream(trade_map_td const &tradeMap) {
  //
}

void processCandlestickStream(trade_map_td const &tradeMap) {
  //
}

void processTickerStream(trade_map_td const &tradeMap) {
  //
}

void processDepthStream(trade_map_td const &tradeMap) {
  auto sorter =
      [&tradeMap](
          char const *str) -> std::optional<data_streamer_t<depth_data_t>> {
    if (auto iter = tradeMap.find(str); iter != tradeMap.end()) {
      auto &list = iter->second;
      std::sort(
          list.begin(), list.end(),
          [](std::filesystem::path const &a, std::filesystem::path const &b) {
            return std::filesystem::last_write_time(a) <
                   std::filesystem::last_write_time(b);
          });
      return data_streamer_t<depth_data_t>(list);
    }
    return std::nullopt;
  };

  auto const spotStreamer = sorter(SPOT);
  auto const futuresStreamer = sorter(FUTURES);
}

int main(int argc, char **argv) {
  CLI::App app{"backtesting software for Creed & Bear LLC"};

  stringlist_t streams;
  stringlist_t tradeTypes;
  std::string rootDir;
  std::string dateFromStr;
  std::string dateToStr;
  std::string token;

  app.add_option("--token", token,
                 "token pair [e.g. btcusdt(default), ethusdt]");
  app.add_option("--streams", streams,
                 "A list of the streams(s) to run. Valid options are: "
                 "[trade, ticker, bookticker, depth, kline(default)]");
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
            << "rootDir: " << rootDir << "\n"
            << "token: " << token << std::endl;
  for (auto const &s : streams)
    std::cout << "stream: " << s << std::endl;
  for (auto const &t : tradeTypes)
    std::cout << "trade: " << t << std::endl;

#endif // _DEBUG

  if (streams.empty()) {
    streams.push_back(CANDLESTICK);
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

  if (token.empty())
    token = "BTCUSDT";

  if (rootDir.empty())
    rootDir = ".";

  if (!std::filesystem::exists(rootDir)) {
    std::cerr << "'" << rootDir << "' does not exist" << std::endl;
    return EXIT_FAILURE;
  }

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
      tradeTypes, streams, startTime, endTime, rootDir, token);
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
      processCandlestickStream(csData);
    }}.detach();
  }

  if (auto const iter = csvFiles.find(DEPTH); iter != csvFiles.cend()) {
    std::thread{[csData = iter->second] {
      processDepthStream(csData);
    }}.detach();
  }

  /*
  csv::CSVReader reader(filename);
  for (auto const &row : reader) {
    for (auto const &field : row) {
      std::cout << field.get_sv() << ", ";
    }
    std::endl(std::cout);
  }
  */
  return 0;
}
