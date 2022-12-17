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
#define ORDERBOOK "orderBook"
#define SPOT "spot"
#define FUTURES "futures"

using stringlist_t = std::vector<std::string>;
using fs_pathlist_t = std::vector<std::filesystem::path>;

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

fs_pathlist_t
getListOfCSVFiles(stringlist_t const &tradeTypes, stringlist_t const &streams,
                  std::time_t const startTime, std::time_t const endTime,
                  std::string const &rootDir, std::string const &token) {
  std::filesystem::path const rootPath(rootDir);
  auto const intervals = intervalsBetweenDates(startTime, endTime);
  auto const tokenName = toUpperString(token);

  fs_pathlist_t paths;
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
            paths.push_back(file);
        }
      }
    }
  }
  return paths;
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
                 "[trade, ticker, bookticker, kline(default)]");
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
                                                CANDLESTICK};
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

  auto const csvFiles = getListOfCSVFiles(tradeTypes, streams, startTime,
                                          endTime, rootDir, token);
  if (csvFiles.empty()) {
    std::cerr << "No files found matching that criteria" << std::endl;
    return EXIT_FAILURE;
  }
  std::string const filename = csvFiles[0].string();
  std::cout << "Filename: " << filename << std::endl;
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
