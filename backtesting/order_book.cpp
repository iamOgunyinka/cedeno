#include "order_book.hpp"

namespace backtesting {

static int64_t orderNumber = 0x2'300'820'000;

bool isGreater(depth_data_t::depth_meta_t const &a,
               depth_data_t::depth_meta_t const &b) {
  return a.priceLevel > b.priceLevel;
}
bool isLesser(depth_data_t::depth_meta_t const &a,
              depth_data_t::depth_meta_t const &b) {
  return a.priceLevel < b.priceLevel;
}

static auto lesserComparator = [](auto const &a, auto const &b) {
  using type_t = typename std::remove_cv_t<std::decay_t<decltype(a)>>;
  if constexpr (std::is_same_v<type_t, double>)
    return a < b.priceLevel;
  else
    return a.priceLevel < b;
};

static auto greaterComparator = [](auto const &a, auto const &b) {
  using type_t = typename std::remove_cv_t<std::decay_t<decltype(a)>>;
  if constexpr (std::is_same_v<type_t, double>)
    return a > b.priceLevel;
  else
    return a.priceLevel > b;
};

static auto updateSidesWithNewData =
    [](std::vector<depth_data_t::depth_meta_t> const &src,
       std::vector<depth_data_t::depth_meta_t> &dest, auto const comparator) {
      for (auto const &d : src) {
        auto iter = std::lower_bound(dest.begin(), dest.end(), d.priceLevel,
                                     comparator);
        if (iter != dest.end() && iter->priceLevel == d.priceLevel) {
          if (d.quantity == 0.0) {
            dest.erase(iter);
            continue;
          } else {
            iter->quantity += d.quantity;
          }
        } else {
          dest.insert(iter, d);
        }
      }
      dest.erase(
          std::remove_if(dest.begin(), dest.end(),
                         [](auto const &a) { return a.quantity == 0.0; }),
          dest.end());
    };

order_book_t::order_book_t(net::io_context &ioContext,
                           data_streamer_t<depth_data_t> dataStreamer)
    : m_ioContext(ioContext), m_dataStreamer(std::move(dataStreamer)),
      m_currentBook(m_dataStreamer.getNextData()),
      m_currentTimer(m_currentBook.eventTime) {
  std::sort(m_currentBook.asks.begin(), m_currentBook.asks.end(), isLesser);
  std::sort(m_currentBook.bids.begin(), m_currentBook.bids.end(), isGreater);
}

order_book_t::~order_book_t() {
  if (m_periodicTimer) {
    m_periodicTimer->cancel();
    m_periodicTimer.reset();
  }
}

void order_book_t::run() {
  if (m_periodicTimer) // already running
    return;

  setNextTimer();
}

int64_t order_book_t::match(user_order_request_t const &order) {
  static auto marketMatcher = [](std::vector<depth_data_t::depth_meta_t> &list,
                                 double quantity) {
    if (list.empty())
      return -1;
    while (quantity > 0.0 && !list.empty()) {
      auto &front = list.front();
      if (front.quantity > quantity) {
        front.quantity -= quantity;
        quantity = 0.0;
      } else if (front.quantity == quantity) {
        quantity = front.quantity = 0.0;
        list.erase(list.begin());
      } else {
        quantity -= front.quantity;
        list.erase(list.begin());
      }
    }
    return 0;
  };

  auto const isBuying = (order.side == trade_side_e::buy);
  auto &bids = m_currentBook.bids;
  auto &asks = m_currentBook.asks;
  double const qty = order.quantity;

  if (order.market == trade_market_e::market) {
    [[maybe_unused]] auto const status =
        marketMatcher(isBuying ? asks : bids, qty);
  } else if (order.market == trade_market_e::limit) {
    if (isBuying) {
      if (asks.empty()) {
        depth_data_t::depth_meta_t temp{order.priceLevel, qty};
        updateSidesWithNewData({std::move(temp)}, bids, greaterComparator);
        return orderNumber++;
      }

    }
  }

  return orderNumber++;
}

void order_book_t::setNextTimer() {
  if (!m_periodicTimer)
    m_periodicTimer.reset(new net::deadline_timer(m_ioContext));

  m_nextData = m_dataStreamer.getNextData();

  // no more data but we need to keep the simulator running
  if (m_nextData.asks.empty() && m_nextData.bids.empty())
    m_nextData.eventTime = m_currentTimer + 1'000;

  auto const timeDiff = m_nextData.eventTime - m_currentTimer;
#ifdef _DEBUG
  std::cout << "TimeDiff: " << timeDiff << std::endl;
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

#ifdef _DEBUG
void order_book_t::printOrderBook() {
  for (auto const &ask : m_currentBook.asks)
    std::cout << ask.priceLevel << " -> " << ask.quantity << std::endl;
  std::cout << "================" << std::endl;
  for (auto const &bid : m_currentBook.bids)
    std::cout << bid.priceLevel << " -> " << bid.quantity << std::endl;
  std::cout << "<<========END========>>" << std::endl << std::endl;
}
#endif

void order_book_t::updateOrderBook(depth_data_t &&newestData) {
  updateSidesWithNewData(newestData.asks, m_currentBook.asks, lesserComparator);
  updateSidesWithNewData(newestData.bids, m_currentBook.bids,
                         greaterComparator);

#ifdef _DEBUG
  // printOrderBook();
#endif // _DEBUG
}
} // namespace backtesting
