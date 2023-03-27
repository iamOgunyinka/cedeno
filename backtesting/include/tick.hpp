#pragma once

#include <boost/asio/io_context.hpp>
#include <memory>

#ifdef BT_USE_WITH_INDICATORS
#include <boost/asio/deadline_timer.hpp>
#include <set>

#include "indicator_data.hpp"
#endif

namespace net = boost::asio;

namespace backtesting {

#ifdef BT_USE_WITH_INDICATORS

class tick_t : public std::enable_shared_from_this<tick_t> {
public:
  static std::shared_ptr<tick_t> instance();

  ~tick_t();
  void setCallback(indicator_callback_t func) { m_callback = func; }
  void addTicks(std::vector<size_t> const &ticks);
  inline void
  addIndicators(std::vector<indicator_metadata_t *> const &indicators) {
    m_allIndicators.clear();
    m_allIndicators = indicators;
  }

  inline void addTick(size_t const tick) {
    if (auto const &[_, inserted] = m_ticks.insert(tick); inserted)
      createTimerWithTick(tick);
  }

  void stopAllTicks();
  void stopTimerWithTick(size_t tick);

private:
  explicit tick_t() : tick_t{std::vector<size_t>{}} {}
  explicit tick_t(std::vector<size_t> const &ticks);
  void onTimerTimedout(size_t, net::deadline_timer *);
  void createTimerWithTick(size_t);

  std::shared_ptr<net::io_context> m_ioContext = nullptr;
  std::vector<net::deadline_timer *> m_timers;
  std::vector<indicator_metadata_t *> m_allIndicators;
  std::set<size_t> m_ticks;
  indicator_callback_t m_callback = nullptr;
};

#endif

std::shared_ptr<net::io_context> getContextObject();
} // namespace backtesting
