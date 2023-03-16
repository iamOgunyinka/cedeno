#pragma once

#include <memory>

#include <boost/asio/deadline_timer.hpp>

#include "Signals/Signal.h"
#include "depth_data.hpp"
#include "user_data.hpp"
#include <algorithm>

#ifdef BT_USE_WITH_INDICATORS
#include "indicators/manager/indc_mnger.hpp"
#endif

namespace boost {
namespace asio {
class io_context;
} // namespace asio
} // namespace boost

namespace net = boost::asio;

namespace backtesting {
class order_book_base_t;
}

namespace matching_engine {
void placeOrder(backtesting::order_book_base_t &orderBook,
                backtesting::order_data_t const &order);
void cancelOrder(backtesting::order_book_base_t &orderBook,
                 backtesting::order_data_t const &order);
} // namespace matching_engine

namespace backtesting {

namespace details {
/// an internal structure used to hold an item on one side of an order book
struct order_book_entry_t {
  double totalQuantity = 0.0;
  double priceLevel = 0.0;
  std::vector<order_data_t> orders;
};

/// an internal structure holding both sides on a order book (asks and bids)
struct order_book_side_t {
  std::vector<order_book_entry_t> bids;
  std::vector<order_book_entry_t> asks;
};

/// an internal structure used for sort order book entry from lower to higher price
struct lesser_comparator_t {
  bool operator()(double const a, order_book_entry_t const &b) const {
    return a < b.priceLevel;
  }

  bool operator()(order_book_entry_t const &a, double const b) const {
    return a.priceLevel < b;
  }
};

/// an internal structure used for sort order book entry from higher to lower price
struct greater_comparator_t {
  bool operator()(double const a, order_book_entry_t const &b) const {
    return a > b.priceLevel;
  }

  bool operator()(order_book_entry_t const &a, double const b) const {
    return a.priceLevel > b;
  }
};

} // namespace details

/// the pure virtual base class of all order books
class order_book_base_t {
public:

  /// place an order on the order book (for a market order, it isn't added to the book)
  /// \param orderBook - an order book that is meant to handle spot or futures trading
  /// \param order - an order to BUY/SELL
  friend void
  matching_engine::placeOrder(backtesting::order_book_base_t &orderBook,
                              backtesting::order_data_t const &order);

  /// cancel an order already on the order book
  /// \param orderBook - an order book that is meant to handle spot or futures trading
  /// \param order - an order to cancel
  friend void
  matching_engine::cancelOrder(backtesting::order_book_base_t &orderBook,
                               backtesting::order_data_t const &order);

  /// the order_book_base_t only constructor
  /// \param ioContext - an IO context object used to initialize the timer
  /// \param dataStreamer - a data stream that gives the order book persisted data
  /// \param symbol - the symbol being traded in this order book
  order_book_base_t(net::io_context &ioContext,
                    data_streamer_t<depth_data_t> dataStreamer,
                    internal_token_data_t *symbol);

  /// the base destructor
  virtual ~order_book_base_t();
  /// an asynchronous function called to kickstart the order book and get it running
  void run();

  /// get the last price of the instrument on the ask side
  /// \return the price
  double currentSellPrice();
  /// get the last price of the instrument on the bids side
  /// \return the price
  double currentBuyPrice();
  /// internal signal emitted when a new trade occurs
  Gallant::Signal1<trade_list_t> NewTradesCreated;
  /// internal price emittance signal
  Gallant::Signal2<internal_token_data_t *, double> NewMarketPrice;

#ifdef BT_USE_WITH_INDICATORS
  inline indicators::indicators_c& indicator() { return m_indicator; }
  inline void setIndicatorConfiguration(std::vector<std::vector<std::string>> && config) {
    m_indicator.set(std::move(config));
  }
#endif

private:
  void updateOrderBook(depth_data_t &&newestData);
  void setNextTimer();
  void cancelOrder(backtesting::order_data_t order);
  void placeOrder(backtesting::order_data_t order);
  void shakeOrderBook();
  [[nodiscard]] virtual trade_list_t
  marketMatcher(std::vector<details::order_book_entry_t> &list,
                double &amountAvailableToSpend, order_data_t const &order);

protected:
#ifdef _DEBUG
  /// print the visible representation of the book
  virtual void printOrderBook() = 0;

#endif

  /// a market matching implementation, the behavior differs in spot and futures
  /// \param list the bids or asks depending on the side of the order
  /// \param amountAvailableToSpend ignored in futures order book but
  /// for spot order book, this is the amount the user is spending on buying or selling
  /// \param order the order data signifying what to buy or sell
  /// \return the list of trades that occurred
  [[nodiscard]] virtual trade_list_t
  marketMatcherImpl(std::vector<details::order_book_entry_t> &list,
                    double &amountAvailableToSpend,
                    order_data_t const &order) = 0;

  /// an internal function used to create a `trade_data_t` object
  /// \param order the data with information about the trade
  /// \param qty the amount sold or bought in the trade
  /// \param amount the price the instrument is traded for
  /// \return a `trade_data_t` for that trade that occurred
  [[nodiscard]] trade_data_t getNewTrade(order_data_t const &order,
                                         order_status_e const, double const qty,
                                         double const amount);
  /// an internal function that is used to notify the other side of a trade,
  /// for example if an order is bought, someone on the other side sold that
  /// instrument. This function returns the trade on the other side
  /// \param data the entry/line in the order book where the trade occurred
  /// \param quantityTraded the amount sold or bought in the trade
  /// \param priceLevel the price the instrument is traded for
  /// \return a (possible) list of all the trades that was bought on the other side
  /// of the trade
  [[nodiscard]] trade_list_t
  getExecutedTradesFromOrders(details::order_book_entry_t &data,
                              double quantityTraded, double const priceLevel);
  /// an IO context object used to control the timers
  net::io_context &m_ioContext;
  //// a stream object responsible for reading depth stream from file to the order book
  data_streamer_t<depth_data_t> m_dataStreamer;
  /// the symbol being traded
  internal_token_data_t *m_symbol = nullptr;
  /// the timer that controls the time between data updates
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  /// the order book entries -- the asks and the bids
  details::order_book_side_t m_orderBook;
  /// the current depth entry
  depth_data_t m_nextData;
  /// the timer object
  time_t m_currentTimer = 0;

#ifdef BT_USE_WITH_INDICATORS
  indicators::indicators_c m_indicator;
#endif
};

details::order_book_entry_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       internal_token_data_t *token, trade_side_e const side,
                       trade_type_e const tradeType);
/// the progressive order number
/// \return the next order number
int64_t getOrderNumber();

} // namespace backtesting
