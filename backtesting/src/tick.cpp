#include "tick.hpp"
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#include <spdlog/spdlog.h>

namespace backtesting {

std::shared_ptr<tick_t> tick_t::instance() {
  static std::shared_ptr<tick_t> tickInstance(new tick_t());
  return tickInstance;
}

tick_t::tick_t(std::vector<size_t> const &ticks)
    : m_ioContext(getContextObject()), m_ticks(ticks.begin(), ticks.end()) {
}

tick_t::~tick_t() {
  m_ticks.clear();
  m_indicatorMap.clear();

  for (auto &timer: m_timers)
    delete timer;
  m_timers.clear();
  m_ioContext = nullptr;
  spdlog::info("All instances of this object has been destroyed");
  m_callback = nullptr;
}

void tick_t::stopAllTicks() {
  spdlog::info("Cancelling all ticks");
  boost::system::error_code ec {};
  for (auto& timer: m_timers) {
    timer->cancel(ec);
  }
  spdlog::info("[Done] Cancelling all ticks");
}

void tick_t::stopTimerWithTick(const size_t tick) {
  auto const time = boost::posix_time::seconds(tick);
  for (auto iter = m_timers.begin(); iter != m_timers.end(); ++iter) {
    auto& timer = *iter;
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
  for (auto const tick: ticks)
    addTick(tick);
}

void tick_t::onTimerTimedout(size_t const tick, net::deadline_timer* timer) {
  if (m_callback && m_ioContext) {
    net::post(net::make_strand(*m_ioContext),
              [self = shared_from_this(), timer, tick] {
                auto& data = self->m_indicatorMap[timer];
                data.time = tick;
                self->m_callback(data);
              });
  }

  timer->expires_from_now(boost::posix_time::seconds(tick));
  timer->async_wait([self = shared_from_this(), tick, timer](boost::system::error_code const ec) {
    if (ec == net::error::operation_aborted)
      return;
    self->onTimerTimedout(tick, timer);
  });
}

std::shared_ptr<net::io_context> getContextObject () {
  static auto object = std::make_shared<net::io_context>();
  return object;
}

} // namespace backtesting
