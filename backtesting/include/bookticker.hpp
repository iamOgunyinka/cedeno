#pragma once

#include "common.hpp"
#include "container.hpp"
#include "data_streamer.hpp"
#include <array>
#include <functional>

namespace backtesting {

/// This is an internal structure. It is read from the file and processed
/// internally
struct exchange_bktick_data_t {
  double orderBookUpdateID;     /*!< the order book update ID */
  double bestBidPrice;          /*!< the best bid price */
  double bestBidQty;            /*!< the best bid quantity */
  double bestAskPrice;          /*!< the best ask price */
  double bestAskQty;            /*!< the best ask quantity */
  uint64_t eventTimeInMs;       /*!< the event time in milliseconds */
  uint64_t transactionTimeInMs; /*!< the transaction time in milliseconds */

  /// internal function responsible for reading data from file using the
  /// dataStreamer \param dataStreamer a stream object responsible for reading
  /// data from file \return a `exchange_bktick_data_t` object
  static exchange_bktick_data_t
  dataFromCSVStream(data_streamer_t<exchange_bktick_data_t> &dataStreamer);

private:
  template <typename T> static T getNumber(csv::CSVRow::iterator const &iter) {
    return iter->is_str() ? std::stoull(iter->get_sv().data()) : iter->get<T>();
  }

  static bool isExpectedRowCount(size_t const r) { return r == 7; }
  static exchange_bktick_data_t bookTickerFromCSVRow(csv::CSVRow const &row);
};

/// This is the book ticker data structure returned to the user.
struct bktick_data_t {
  uint64_t ts;         //!< the event time
  double bestBidPrice; //!< the best bid price
  double bestBidQty;   //!< the best bid quantity
  double bestAskPrice; //!< the best ask price
  double bestAskQty;   //!< the best ask quantity
  std::string symbol;  //!< the name of the symbol
};

using bktick_list_t = std::vector<bktick_data_t>;
using bktick_callback_t = std::function<void(bktick_data_t const &)>;

/// This is the configuration sent from the user (most likely from the Python
/// script) specifying the symbol needed, start and end time, trade type and an
/// optional callback to get periodic data
struct bktick_config_t {
  //!< a list of symbols whose book ticker needs to be extracted
  std::vector<std::string> symbols;
  trade_type_e tradeType; //!< trade type -> futures or spot
  time_t startTime = 0;   //!< the time where the data extraction starts
  time_t endTime = 0;     //!< the time where the data extraction ends
  bktick_callback_t callback =
      nullptr; //!< the callback needed for periodic updates

  /// an internal data used to monitor book ticker configurations
  static ::utils::waitable_container_t<bktick_config_t> bookTickerConfigs;
  /// globalBkTickerRecord[0] == spot
  /// globalBkTickerRecord[1] == futures
  static std::array<std::vector<bktick_data_t>, 2> globalBkTickerRecord;
  static std::mutex globalBkTickerMutex;
};

/// getDiscreteBTickerData - A function to return the current book ticker data
/// \param config - a `bktick_config_t` object specifying the user configuration
/// \return a list of book ticker data
bktick_list_t getDiscreteBTickerData(bktick_config_t &&config);
/// checkAndValidateBookTickerRequestHelper - an internal helper function to
/// check that a user configuration is valid \param config - the user
/// configuration \return bool - True if it is a valid configuration, False
/// otherwise
bool checkAndValidateBookTickerRequestHelper(bktick_config_t &config);
/// checkAndValidateBookTickerRequest - an internal function to check that a
/// user configuration is valid \param config - the user configuration \return
/// bool - True if it is a valid configuration, False otherwise
bool checkAndValidateBookTickerRequest(bktick_config_t &config);
/// getContinuousBTickerData - works like websocket, this function allows a user
/// to get book ticker data every N specified time
/// \param config - the user configuration
/// \return bool - True if it is a valid configuration, False otherwise
bool getContinuousBTickerData(bktick_config_t &&config);
/// bookTickerProcessingThreadImpl - an internal function that reads files from
/// disk, process it and does some in-house processing \param config - the user
/// configuration
[[maybe_unused]] void bookTickerProcessingThreadImpl(bktick_config_t &&config);
/// insertNewestData - another internal function, not to be used by the user
void insertNewestData(bktick_data_t const &, bktick_config_t const &);
/// bookTickerChildThreadImpl - an internal function, it is used by a thread to
/// read book ticker from file and preprocess it for further processing
void bookTickerChildThreadImpl(bktick_config_t &&);
} // namespace backtesting
