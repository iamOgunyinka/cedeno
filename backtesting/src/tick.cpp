#include "tick.hpp"
#include <boost/asio/post.hpp>

namespace backtesting {
tick_t &tick_t::tickInstance(net::io_context &ioContext,
                             std::vector<size_t> const &ticks) {
  static tick_t tick(ioContext, ticks);
  return tick;
}

tick_t::tick_t(net::io_context &ioContext, std::vector<size_t> const &ticks)
    : m_ioContext(ioContext) {
  m_timers.reserve(ticks.size());

  for (size_t const tick : ticks) {
    auto timer = std::make_unique<net::deadline_timer>(m_ioContext);
    timer->expires_from_now(boost::posix_time::seconds(tick));
    timer->async_wait([this, tick, t = timer.get()](auto const) {
      return onTimerTimedout(tick, t);
    });
    m_timers.push_back(std::move(timer));
  }
}

void tick_t::onTimerTimedout(size_t const tick, net::deadline_timer *timer) {
  if (m_callback) {
    net::post(m_ioContext.get_executor(),
              [this, timer] { m_callback(m_indicatorMap[timer]); });
  }

  timer->expires_from_now(boost::posix_time::seconds(tick));
  timer->async_wait([this, tick, timer](boost::system::error_code const) {
    onTimerTimedout(tick, timer);
  });
}

} // namespace backtesting
