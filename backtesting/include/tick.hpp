#pragma once

#include <boost/asio/io_context.hpp>
#include <memory>

#ifdef BT_USE_WITH_INDICATORS
#include <boost/asio/deadline_timer.hpp>
#include <map>
#include <set>

#include "indicator_data.hpp"
#endif

namespace net = boost::asio;

namespace backtesting {

#ifdef BT_USE_WITH_INDICATORS

struct timer_metadata_t {
  net::io_context &ioContext;
  net::deadline_timer *timer = nullptr;
  size_t tick = 0;
  timeframe_info_t timeInfo;

  explicit timer_metadata_t(net::io_context &ioContext_, size_t const tick_)
      : ioContext(ioContext_), tick(tick_) {}
};

class ticker_t : public std::enable_shared_from_this<ticker_t> {
public:
  static std::shared_ptr<ticker_t> instance();

  ~ticker_t();
  void setCallback(indicator_callback_t func) { m_callback = func; }
  inline void addTicks(std::vector<size_t> const &ticks) {
    m_ticks.insert(ticks.cbegin(), ticks.cend());
  }

  inline void
  addIndicators(std::vector<indicator_metadata_t *> const &indicators) {
    m_allIndicators.clear();
    m_allIndicators = indicators;
  }

  void stopTimers();
  void startTimers();

private:
  ticker_t();
  void onPeriodicTimerTimedOut();
  void onTickTimersTimedOut(size_t, timer_metadata_t *);
  void startOtherTimers();
  void setupNextTimeTick(size_t, timer_metadata_t *);

  std::shared_ptr<net::io_context> m_ioContext = nullptr;
  std::unique_ptr<net::deadline_timer> m_periodicTimer = nullptr;
  std::vector<timer_metadata_t *> m_timers;
  std::set<size_t> m_ticks;
  std::vector<indicator_metadata_t *> m_allIndicators;
  indicator_callback_t m_callback = nullptr;
};

#endif

std::shared_ptr<net::io_context> getContextObject();
} // namespace backtesting
