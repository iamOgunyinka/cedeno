#include "order_book_base.hpp"

#ifdef _DEBUG
#include <spdlog/spdlog.h>
#endif

namespace backtesting {
using simple_sort_callback_t = bool (*)(details::order_meta_data_t const &,
                                        details::order_meta_data_t const &);

int64_t getOrderNumber() {
  static int64_t orderNumber = 0x0'100'120'000;
  static std::mutex mutex;

  std::lock_guard<std::mutex> lock_g(mutex);
  if ((orderNumber + 1) >= std::numeric_limits<int64_t>::max())
    orderNumber = 0x0'100'120'000;
  return orderNumber++;
}

bool isGreater(details::order_meta_data_t const &a,
               details::order_meta_data_t const &b) {
  return a.priceLevel > b.priceLevel;
}

bool isLesser(details::order_meta_data_t const &a,
              details::order_meta_data_t const &b) {
  return a.priceLevel < b.priceLevel;
}

details::order_meta_data_t
orderMetaDataFromDepth(depth_data_t::depth_meta_t const &depth,
                       internal_token_data_t *token, trade_side_e const side,
                       trade_type_e const tradeType) {
  details::order_meta_data_t d;
  d.totalQuantity += depth.quantity;
  d.priceLevel = depth.priceLevel;

  order_data_t order;
  order.market = market_type_e::limit;
  order.priceLevel = depth.priceLevel;
  order.quantity = depth.quantity;
  order.side = side;
  order.type = tradeType;
  order.token = token;
  order.orderID = getOrderNumber();
  d.orders.push_back(order);
  return d;
}

void insertAndSort(std::vector<depth_data_t::depth_meta_t> const &depthList,
                   std::vector<details::order_meta_data_t> &dst,
                   trade_side_e const side, trade_type_e const tradeType,
                   internal_token_data_t *token,
                   simple_sort_callback_t comparator) {
  for (auto const &depth : depthList)
    dst.push_back(orderMetaDataFromDepth(depth, token, side, tradeType));

  std::sort(dst.begin(), dst.end(), comparator);
}

order_book_base_t::order_book_base_t(net::io_context &ioContext,
                                     data_streamer_t<depth_data_t> dataStreamer,
                                     internal_token_data_t *symbol,
                                     trade_type_e const tradeType)
    : m_ioContext(ioContext), m_dataStreamer(std::move(dataStreamer)),
      m_symbol(symbol), m_tradeType(tradeType) {
  auto firstData = m_dataStreamer.getNextData();
  firstData.tradeType = m_tradeType;

  insertAndSort(firstData.bids, m_orderBook.bids, trade_side_e::buy,
                m_tradeType, symbol, isGreater);
  insertAndSort(firstData.asks, m_orderBook.asks, trade_side_e::sell,
                m_tradeType, symbol, isLesser);
  m_currentTimer = firstData.eventTime;

  NewDepthObtained(firstData);
}

order_book_base_t::~order_book_base_t() {
  if (m_periodicTimer) {
    m_periodicTimer->cancel();
    m_periodicTimer.reset();
  }
}

void order_book_base_t::run() {
  if (m_periodicTimer) // already running
    return;

  setNextTimer();
}

void order_book_base_t::setNextTimer() {
  if (!m_periodicTimer)
    m_periodicTimer.reset(new net::deadline_timer(m_ioContext));

  m_nextData = m_dataStreamer.getNextData();
  m_nextData.tradeType = m_tradeType;
  NewDepthObtained(m_nextData);

  // no more data but we need to keep the simulator running
  if (m_nextData.asks.empty() && m_nextData.bids.empty())
    m_nextData.eventTime = m_currentTimer + 1'000;

  auto const timeDiff = m_nextData.eventTime - m_currentTimer;
#ifdef _DEBUG
  spdlog::debug("TimeDiff: {}", timeDiff);
  assert(timeDiff > 0);
#else
  if (timeDiff <= 0LL)
    return;
#endif // _DEBUG

  m_currentTimer = m_nextData.eventTime;

  m_periodicTimer->expires_from_now(boost::posix_time::milliseconds(timeDiff));
  m_periodicTimer->async_wait([this](boost::system::error_code const ec) {
    updateOrderBook(std::move(m_nextData));
    setNextTimer();
  });
}

void order_book_base_t::updateOrderBook(depth_data_t &&newestData) {
  using details::greater_comparator_t;
  using details::lesser_comparator_t;

  updateSidesWithNewDepth(newestData.asks, m_orderBook.asks, trade_side_e::sell,
                          m_tradeType, m_symbol, lesser_comparator_t{});
  updateSidesWithNewDepth(newestData.bids, m_orderBook.bids, trade_side_e::buy,
                          m_tradeType, m_symbol, greater_comparator_t{});
  shakeOrderBook();
#ifdef _DEBUG
  printOrderBook();
#endif // _DEBUG
}

} // namespace backtesting
