#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "enumerations.hpp"

namespace backtesting {
using fs_list_t = std::vector<std::filesystem::path>;
using stringlist_t = std::vector<std::string>;
using stream_type_td = std::string;
using trade_type_td = std::string;
using token_map_td = std::map<std::string, backtesting::fs_list_t>;
using trade_map_td = std::map<trade_type_td, token_map_td>;
using filename_map_td = std::map<stream_type_td, trade_map_td>;
enum class trade_type_e;

std::string getDatabaseConfigPath();

namespace utils {

void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);
std::string ltrim_copy(std::string s);
std::string rtrim_copy(std::string s);
std::string trim_copy(std::string s);
void trim_string(std::string &str);
std::string getRandomString(std::size_t const length);
std::size_t getRandomInteger();
char getRandomChar();
std::vector<std::filesystem::path>
listOfFilesForTradeData(time_t const startTime, time_t const endTime,
                        trade_type_e const tt, std::string const &symbol,
                        std::string const &stream);
std::optional<std::time_t> dateStringToTimeT(std::string const &s);
unsigned long timeStringToSeconds(std::string const &s);
std::string toUpperString(std::string const &s);
void removeAllQuotes(std::string &s);
bool listContains(std::vector<std::string> const &container,
                  std::string const &t);
bool startsWith(std::string const &s, std::string const &substr);
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
