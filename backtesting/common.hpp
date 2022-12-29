#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#define CANDLESTICK "kline"
#define BTICKER "bookTicker"
#define TICKER "ticker"
#define TRADE "trade"
#define DEPTH "depth"

#define SPOT "spot"
#define FUTURES "futures"

namespace backtesting {
using fs_list_t = std::vector<std::filesystem::path>;
using stringlist_t = std::vector<std::string>;
using stream_type_td = std::string;
using trade_type_td = std::string;
using token_map_td = std::map<std::string, backtesting::fs_list_t>;
using trade_map_td = std::map<trade_type_td, token_map_td>;
using filename_map_td = std::map<stream_type_td, trade_map_td>;

std::string getDatabaseConfigPath();

namespace utils {

void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);
std::string ltrim_copy(std::string s);
std::string rtrim_copy(std::string s);
std::string trim_copy(std::string s);
void trim_string(std::string &str);

std::optional<std::time_t> stringToTimeT(std::string const &s);
std::string toUpperString(std::string const &s);
bool listContains(std::vector<std::string> const &container,
                  std::string const &t);
std::chrono::seconds stringToStdInterval(std::string &str);
std::vector<std::string> splitString(std::string const &str, char const *delim);
filename_map_td
getListOfCSVFiles(stringlist_t const &tokenList, stringlist_t const &tradeTypes,
                  stringlist_t const &streams, std::time_t const startTime,
                  std::time_t const endTime, std::string const &rootDir);
std::vector<std::time_t> intervalsBetweenDates(std::time_t const start,
                                               std::time_t const end);
std::optional<std::string> currentTimeToString(std::time_t const currentTime,
                                               std::string const &delim);
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
} // namespace utils
} // namespace backtesting
