#pragma once

#include "bookticker.hpp"
#include "candlestick_data.hpp"

#ifdef BT_USE_WITH_INDICATORS
namespace indicator {
bool isValidIndicatorConfiguration(
    std::vector<std::vector<std::string>> const &);
}
#endif

namespace backtesting {
enum class trade_type_e;

#ifdef BT_USE_WITH_DB
std::string getDatabaseConfigPath();
#endif // BT_USE_WITH_DB

///  allows the user to configure the backtest instance
struct configuration_t {
  using stringlist_t = std::vector<std::string>;

#ifdef BT_USE_WITH_INDICATORS
  /// a list of configurations used by the indicators
  std::vector<std::vector<std::string>> indicatorConfig;
  /// a list of time frames where indicator data is sent to the user
  std::vector<size_t> ticks;
#endif

  /// a list of the streams(s) to run. Valid options are:
  /// trade, ticker, bookticker, kline, depth(default)
  stringlist_t streams;
  /// a list of trade types. Valid options are: futures, spot(default)
  stringlist_t tradeTypes;
  /// a list of valid symbols e.g BTCUSDT,ETHUSDT,RUNEUSDT
  stringlist_t tokenList;
  /// a string specifying the date from which the backtesting
  /// instance is started. Format: yyyy-mm-dd HH:MM:SS
  std::string dateFromStr;
  /// a string specifying the date from which the backtesting
  /// instance is stopped. Format: yyyy-mm-dd HH:MM:SS
  std::string dateToStr;
  /// Root directory where the historical data are stored
  /// (default: the current working directory `pwd`)
  std::string rootDir;
  /// the maker fee rate in percentage for futures trading
  double futuresMakerFee = 0.02;
  /// the taker fee rate in percentage for futures trading
  double futuresTakerFee = 0.04;
  /// the maker fee rate in percentage for spot trading
  double spotMakerFee = 0.1;
  /// the taker fee rate in percentage for spot trading
  double spotTakerFee = 0.1;
  /// show logs on every operation performed by backtest
  bool verbose = false;

  /**
   * klineConfig specifies a configuration for receiving continuous kline data.
   * This allows the user get periodic kline updates as obtained from the
   * exchanges. It is optional and be left out completely
   */
  std::optional<kline_config_t> klineConfig = std::nullopt;

  /**
   * bookTickerConfig specifies a configuration for receiving continuous book
   * ticker data. This allows the user get periodic book ticker updates as
   * obtained from the exchanges. It is optional and be left out completely
   */
  std::optional<bktick_config_t> bookTickerConfig = std::nullopt;

#ifdef BT_USE_WITH_DB
  std::string dbConfigFilename = getDatabaseConfigPath();
  std::string dbLaunchType = "development";
#endif
};

/*
void readTokensFromFileImpl(token_data_list_t& result,
                            trade_type_e const tradeType,
                            std::string const &filename);
*/
} // namespace backtesting

/// provides the core functionality for users of the back testing project.
/*! there must be only one instance of this class */
class backtesting_t {
  /// @private isReady - Checks if a backtesting instance is ready to be run
  ///
  /// @param void
  /// \return bool
  bool isReady() const { return m_argumentParsed && m_authenticatedData; }

  /// @private parseImpl - checks and ensures that the configuration passed from
  /// the user is valid. This function is called before `prepareData` and
  /// `isReady`
  /// @param `backtesting::configuration_t` - An instance of the
  /// `configuration_t` class
  /// @return bool - True on success, False on failure
  bool parseImpl(backtesting::configuration_t);
  void reset();

public:
  backtesting_t();

  /// the ctor that takes an instance of `configuration_t`
  backtesting_t(backtesting::configuration_t const &);

  /// parse - parses the command line arguments
  /// \param argc - the size of `argv`
  /// \param argv - a list of options including the process name
  /// \return bool - True on success, False on failure
  bool parse(int argc, char **argv);

  /// prepares the back testing instance configuration data
  /// \return bool - True on success, False on failure
  bool prepareData();

  /// Asynchronously run the instance of the back testing
  /// \return 0 on success and non-zero on failure
  int run();

  friend backtesting_t *newBTInstance(backtesting::configuration_t const &);

private:
  std::optional<backtesting::configuration_t> m_config;
  bool m_argumentParsed = false;
  bool m_authenticatedData = false;
};

/// creates an instance of backtesting class using the `configuration_t` object
/// @param config - a valid `configuration_t` object
/// \return a valid backtesting_t instance on success and std::nullopt on
/// failure
backtesting_t *newBTInstance(backtesting::configuration_t const &);

namespace backtesting {
class user_data_t;

std::unique_ptr<backtesting_t> &getGlobalBTInstance();
bool createBTInstanceFromConfigFile(std::string const &filename);
user_data_t *getGlobalUser();
} // namespace backtesting
