#include "common.hpp"
#include <algorithm>
#include <cstring>
#include <random>
#include <sstream>

namespace backtesting {
std::string getDatabaseConfigPath() {
  return (std::filesystem::current_path()
#ifdef _DEBUG
          / "scripts"
#endif // _DEBUG
          / "config" / "database.ini")
      .string();
}

namespace utils {
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

void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

std::string ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

std::string rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

void trim_string(std::string &str) { trim(str); }

std::size_t getRandomInteger() {
  static std::random_device rd{};
  static std::mt19937 gen{rd()};
  static std::uniform_int_distribution<> uid(1, 50);
  return uid(gen);
}

char getRandomChar() {
  static std::random_device rd{};
  static std::mt19937 gen{rd()};
  static std::uniform_int_distribution<> uid(0, 52);
  static char const *all_alphas =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  return all_alphas[uid(gen)];
}

std::string getRandomString(std::size_t const length) {
  std::string result{};
  result.reserve(length);
  for (std::size_t i = 0; i != length; ++i)
    result.push_back(getRandomChar());
  return result;
}

bool listContains(std::vector<std::string> const &container,
                  std::string const &t) {
  for (auto const &data : container)
    if (isCaseInsensitiveStringCompare(data, t))
      return true;
  return false;
}

bool startsWith(std::string const &s, std::string const &substr) {
  return s.rfind(substr, 0) == 0;
}

std::optional<std::time_t> dateStringToTimeT(std::string const &s) {
  time_t tStart;

  int yy, month, dd, hh, mm, ss;
  struct tm whenStart;
  char const *zStart = s.c_str();

  if (sscanf(zStart, "%d-%d-%d %d:%d:%d", &yy, &month, &dd, &hh, &mm, &ss) < 0)
    return std::nullopt;
  whenStart.tm_year = yy - 1'900;
  whenStart.tm_mon = month - 1;
  whenStart.tm_mday = dd;
  whenStart.tm_hour = hh;
  whenStart.tm_min = mm;
  whenStart.tm_sec = ss;
  whenStart.tm_isdst = -1;

  tStart = mktime(&whenStart);
  return tStart;
}

unsigned long timeStringToSeconds(std::string const &s) {
  int second = 0, minute = 0, hour = 0;
  if (sscanf(s.c_str(), "%d_%d_%d", &hour, &minute, &second) < 0)
    return 0;
  unsigned long const result = (hour * 3'600) + (minute * 60) + (second);
  return result;
}

std::string toUpperString(std::string const &s) {
  std::string temp;
  auto const strLen = s.size();
  temp.resize(strLen);
  for (std::string::size_type i = 0; i < strLen; ++i)
    temp[i] = std::toupper(s[i]);
  return temp;
}

void removeAllQuotes(std::string &str) {
  str.erase(std::remove_if(str.begin(), str.end(),
                           [](char const ch) { return ch == '"'; }),
            str.end());
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

std::vector<std::time_t> intervalsBetweenDates(std::time_t const start,
                                               std::time_t const end) {
  std::time_t const endOfStartDate =
      dateStringToTimeT(currentTimeToString(start, "-").value() + " 23:59:59")
          .value();
  constexpr size_t const secondsInOneDay = 3'600 * 24;
  std::vector<std::time_t> timeList{start};
  if (endOfStartDate > end) { // same day
    timeList.push_back(end);
    return timeList;
  }
  auto s = endOfStartDate;
  while (s <= end) {
    timeList.push_back(s);
    s += secondsInOneDay;
  }

  if (timeList.back() < end)
    timeList.push_back(end);
  return timeList;
}

filename_map_td
getListOfCSVFiles(stringlist_t const &tokenList, std::string const &tradeType,
                  stringlist_t const &streams, std::time_t const startTime,
                  std::time_t const endTime, std::string const &rootDir) {
  std::filesystem::path const rootPath(rootDir);
  auto const intervals = intervalsBetweenDates(startTime, endTime);

  filename_map_td paths;
  for (auto const &interval : intervals) {
    auto const date = currentTimeToString(interval, "_").value();
    for (auto const &streamType : streams) {
      for (auto const &token : tokenList) {
        auto const tokenName = toUpperString(token);
        auto const fullPath =
            rootPath / tokenName / date / streamType / tradeType;
        if (!std::filesystem::exists(fullPath))
          continue;
        for (auto const &file :
             std::filesystem::recursive_directory_iterator(fullPath)) {
          if (std::filesystem::is_regular_file(file)) {
            auto &fileList = paths[streamType][tokenName][tradeType];
            if (std::find(fileList.cbegin(), fileList.cend(), file) ==
                fileList.cend())
              fileList.push_back(file);
          }
        }
      }
    }
  }
  return paths;
}

std::vector<std::string> splitString(std::string const &str,
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
} // namespace utils
} // namespace backtesting
