#pragma once
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <string>

namespace binance {

enum class trade_type_e {
  spot,
  futures,
};

enum class time_type_e {
  time,
  date,
};

struct url_t {
  static char const *const spotHostName;
  static char const *const futuresHostName;
  static char const *const spotPortNumber;
  static char const *const futuresPortNumber;
};

struct internal_token_data_t {
  std::string tokenName;
  bool subscribedFor = false;
};

struct candlestick_data_t {
  std::string tokenName;
  std::string interval;
  uint64_t eventTime;
  uint64_t startTime;
  uint64_t closeTime;
  uint64_t firstTradeID;
  uint64_t lastTradeID;
  std::string openPrice;
  std::string closePrice;
  std::string highPrice;
  std::string lowPrice;
  std::string baseAssetVolume;
  std::string quoteAssetVolume;   // Quote asset volume
  std::string tbBaseAssetVolume;  // Taker buy base asset volume
  std::string tbQuoteAssetVolume; // Taker buy quote asset volume
  size_t numberOfTrades;
  bool klineIsClosed;
};

class locked_file_t {
  std::mutex m_mutex;
  std::unique_ptr<std::ofstream> m_file = nullptr;

public:
  locked_file_t() = default;
  locked_file_t(std::string const &filename)
      : m_file(new std::ofstream(filename, std::ios::out)) {
    if (!(*m_file))
      throw std::runtime_error("unable to open file");
  }

  locked_file_t(locked_file_t &&t) noexcept
      : m_mutex{}, m_file(std::move(t.m_file)) {}
  locked_file_t &operator=(locked_file_t &&t) noexcept {
    if (m_file)
      m_file->close();
    m_file = std::move(t.m_file);
    return *this;
  }

  bool isOpen() const { return m_file && m_file->is_open(); }
  void close() {
    std::lock_guard<std::mutex> lock_g(m_mutex);
    if (m_file)
      m_file->close();
    m_file.reset();
  }

  void flush() {
    std::lock_guard<std::mutex> lock_g(m_mutex);
    if (m_file)
      (*m_file) << std::flush;
  }

  bool changeFilename(std::string const &filename) {
    std::lock_guard<std::mutex> lock_g(m_mutex);
    if (m_file)
      m_file->close();

    m_file.reset(new std::ofstream(filename, std::ios::out));
    if (!(*m_file)) {
      m_file.reset();
      return false;
    }
    return true;
  }

  template <typename T>
  friend std::ostream &operator<<(locked_file_t &f, T const &value) {
    std::lock_guard<std::mutex> lock_g(f.m_mutex);
    return ((*f.m_file) << value);
  }

  ~locked_file_t() {
    if (m_file)
      m_file->close();
    m_file.reset();
  }
};

struct token_filename_map_t {
  using token_name_td = std::string;
  std::map<token_name_td, locked_file_t> dataMap;
};

std::string toLowerString(std::string const &s);
std::string toUpperString(std::string const &s);
std::optional<candlestick_data_t> parseCandleStickData(char const *str,
                                                       size_t const size);
std::optional<std::string> currentTimeToString(time_type_e const);

} // namespace binance
