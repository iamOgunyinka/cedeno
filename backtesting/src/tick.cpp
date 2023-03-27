#include "tick.hpp"

#ifdef BT_USE_WITH_INDICATORS
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#endif

namespace backtesting {

#ifdef BT_USE_WITH_INDICATORS
std::shared_ptr<tick_t> tick_t::instance() {
  static std::shared_ptr<tick_t> tickInstance(new tick_t());
  return tickInstance;
}

tick_t::tick_t(std::vector<size_t> const &ticks)
    : m_ioContext(getContextObject()), m_ticks(ticks.begin(), ticks.end()) {}

tick_t::~tick_t() {
  m_ticks.clear();
  m_allIndicators.clear();

  for (auto &timer : m_timers)
    delete timer;
  m_timers.clear();
  m_ioContext = nullptr;
  m_callback = nullptr;
}

void tick_t::stopAllTicks() {
  boost::system::error_code ec{};
  for (auto &timer : m_timers)
    timer->cancel(ec);
}

void tick_t::stopTimerWithTick(const size_t tick) {
  auto const time = boost::posix_time::seconds(tick);
  for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
    auto &timer = *iter;
    if (timer->expires_from_now() == time) {
      timer->cancel();
      m_timers.erase(iter);
      return;
    }
  }
}

void tick_t::createTimerWithTick(size_t const tick) {
  m_timers.reserve(m_ticks.size() + 1);

  auto timer = new net::deadline_timer(*m_ioContext);
  timer->expires_from_now(boost::posix_time::seconds(tick));
  timer->async_wait([self = shared_from_this(), tick, timer](auto const ec) {
    if (ec == net::error::operation_aborted)
      return;
    self->onTimerTimedout(tick, timer);
  });
  m_timers.push_back(timer);
}

void tick_t::addTicks(std::vector<size_t> const &ticks) {
  for (auto const tick : ticks)
    addTick(tick);
}

void tick_t::onTimerTimedout(size_t const tick, net::deadline_timer *timer) {
  if (m_callback && m_ioContext) {
    indicator_result_t result;
    result.reserve(m_allIndicators.size());
    for (auto &indicator : m_allIndicators) {
      indicator_data_t data;
      data.indicator = indicator->indicator.get();
      data.time = tick;
      data.tradeType = indicator->symbol->tradeType;
      data.symbol = indicator->symbol->name;
      result.push_back(std::move(data));
    }
    net::post(net::make_strand(*m_ioContext),
              [data = std::move(result), self = shared_from_this()] {
                self->m_callback(data);
              });
  }

  timer->expires_from_now(boost::posix_time::seconds(tick));
  timer->async_wait([self = shared_from_this(), tick,
                     timer](boost::system::error_code const ec) {
    if (ec == net::error::operation_aborted)
      return;
    self->onTimerTimedout(tick, timer);
  });
}

#endif

std::shared_ptr<net::io_context> getContextObject() {
  static auto object = std::make_shared<net::io_context>();
  return object;
}

} // namespace backtesting
