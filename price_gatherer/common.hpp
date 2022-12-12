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
  bool m_isWritingHeader = false;

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
  bool rewriteHeader() const { return m_isWritingHeader; }
  void rewriteHeader(bool const v) { m_isWritingHeader = v; }
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

  template <typename... Args> void write(Args &&...args) {
    std::lock_guard<std::mutex> lock_g(m_mutex);
    writeImpl(std::forward<Args>(args)...);
  }

  ~locked_file_t() {
    if (m_file)
      m_file->close();
    m_file.reset();
  }

private:
  template <class T, class... Args>
  void writeImpl2(T const &value, Args &&...args) {
    ((*m_file << args << ", "), ...);
    *m_file << value << "\n";
  }

  template <size_t... I0s, size_t... I1s, class... Ts>
  void writeImpl1(std::index_sequence<I0s...>, // first args
                  std::index_sequence<I1s...>, // last args
                  std::tuple<Ts...> args       // all args
  ) {
    writeImpl2(std::get<I0s>(std::move(args))..., // get first args
               std::get<I1s>(std::move(args))...  // and last args
    );
  }

  template <class... Ts> inline void writeImpl(Ts &&...ts) {
    writeImpl1(std::index_sequence<sizeof...(ts) - 1>{}, // last shall be first
               std::make_index_sequence<sizeof...(ts) - 1>{}, // first be last
               std::forward_as_tuple(std::forward<Ts>(ts)...));
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
