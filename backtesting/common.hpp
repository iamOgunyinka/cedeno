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

enum class trade_type_e : int {
  spot,
  futures,
};

enum class trade_side_e : int {
  sell,
  buy,
};

enum class trade_market_e : int {
  limit,
  market,
};

struct user_order_request_t {
  std::string tokenName;
  double quantity = 0.0;
  double priceLevel = 0.0;
  double leverage = 1.0;

  trade_side_e side;
  trade_type_e type;
  trade_market_e market;
};

inline namespace utils {
using fs_list_t = std::vector<std::filesystem::path>;
using stringlist_t = std::vector<std::string>;
using stream_type_td = std::string;
using trade_type_td = std::string;
using token_map_td = std::map<std::string, backtesting::fs_list_t>;
using trade_map_td = std::map<trade_type_td, token_map_td>;
using filename_map_td = std::map<stream_type_td, trade_map_td>;

std::optional<std::time_t> stringToTimeT(std::string const &s);
std::string toUpperString(std::string const &s);
bool listContains(std::vector<std::string> const &container,
                  std::string const &t);
std::chrono::seconds stringToStdInterval(std::string &str);
std::vector<std::string> split_string(std::string const &str,
                                      char const *delim);
filename_map_td
getListOfCSVFiles(stringlist_t const &tokenList, stringlist_t const &tradeTypes,
                  stringlist_t const &streams, std::time_t const startTime,
                  std::time_t const endTime, std::string const &rootDir);
std::vector<std::time_t> intervalsBetweenDates(std::time_t start,
                                               std::time_t end);
std::optional<std::string> currentTimeToString(std::time_t const currentTime,
                                               std::string const &delim);
bool isCaseInsensitiveStringCompare(std::string const &s, std::string const &t);
} // namespace utils

int64_t initiateOrder(user_order_request_t const &order);

} // namespace backtesting
