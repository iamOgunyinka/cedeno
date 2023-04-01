#include "tick.hpp"

#ifdef BT_USE_WITH_INDICATORS
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#endif

namespace backtesting {

#ifdef BT_USE_WITH_INDICATORS
std::shared_ptr<ticker_t> ticker_t::instance() {
  static std::shared_ptr<ticker_t> tickInstance(new ticker_t());
  return tickInstance;
}

ticker_t::ticker_t() : m_ioContext(getContextObject()) {}

ticker_t::~ticker_t() {
  m_ticks.clear();
  m_allIndicators.clear();
  stopTimers();

  m_ioContext = nullptr;
  m_callback = nullptr;
}

void ticker_t::stopTimers() {
  if (!m_periodicTimer)
    return;

  {
    boost::system::error_code ec{};
    m_periodicTimer->cancel(ec);
    m_periodicTimer.reset();
  }

  for (auto &timerMeta : m_timers) {
    boost::system::error_code ec{};
    timerMeta->timer->cancel(ec);
    delete timerMeta->timer;
    delete timerMeta;
  }
  m_timers.clear();
}

void ticker_t::startTimers() {
  if (m_callback == nullptr || m_ioContext == nullptr || m_periodicTimer ||
      m_ticks.empty())
    return;

  startOtherTimers();

  m_periodicTimer = std::make_unique<net::deadline_timer>(*m_ioContext);
  m_periodicTimer->expires_from_now(boost::posix_time::millisec(BT_MILLI));
  m_periodicTimer->async_wait([self = shared_from_this()](auto const ec) {
    if (ec == net::error::operation_aborted)
      return;
    self->onPeriodicTimerTimedOut();
  });
}

void ticker_t::setupNextTimeTick(size_t const tick,
                                 timer_metadata_t *timerMeta) {
  auto &timer = timerMeta->timer;

  if (!timer)
    timer = new net::deadline_timer(timerMeta->ioContext);

  timer->expires_from_now(boost::posix_time::seconds(tick));
  timer->async_wait(
      [self = shared_from_this(), tick, timerMeta](auto const ec) {
        if (ec == net::error::operation_aborted)
          return;
        self->onTickTimersTimedOut(tick, timerMeta);
      });
}

void ticker_t::startOtherTimers() {
  m_timers.clear();
  m_timers.reserve(m_ticks.size());

  for (auto const tick : m_ticks) {
    auto timer = new timer_metadata_t(*m_ioContext, tick);
    setupNextTimeTick(tick, timer);
    m_timers.push_back(timer);
  }
}

void ticker_t::onTickTimersTimedOut(size_t const tick,
                                    timer_metadata_t *timerMeta) {
  timerMeta->timeInfo.isClosed = true;
  setupNextTimeTick(tick, timerMeta);
}

void ticker_t::onPeriodicTimerTimedOut() {
  {
    for (auto &indicator : m_allIndicators) {
      auto data = indicator->indicator.get();
      auto const &symbol = indicator->symbol;
      for (auto &timerMeta : m_timers) {
        auto &timerInfo = timerMeta->timeInfo;
        timerMeta->timeInfo.dataMap[symbol].push_back(data);
      }
    }

    net::post(net::make_strand(*m_ioContext), [self = shared_from_this()] {
      indicator_data_t result;
      for (auto &timerMeta : self->m_timers) {
        std::string const timeString = std::to_string(timerMeta->tick);
        auto &timeInfo = timerMeta->timeInfo;
        result[timeString] = timeInfo;
        if (timeInfo.isClosed) {
          timeInfo.dataMap.clear();
          timeInfo.isClosed = false;
        }
      }
      self->m_callback(std::move(result));
    });
  }

  m_periodicTimer->expires_from_now(boost::posix_time::milliseconds(BT_MILLI));
  m_periodicTimer->async_wait(
      [self = shared_from_this()](boost::system::error_code const ec) {
        if (ec == net::error::operation_aborted)
          return;
        self->onPeriodicTimerTimedOut();
      });
}

#endif

std::shared_ptr<net::io_context> getContextObject() {
  static auto object = std::make_shared<net::io_context>();
  return object;
}

} // namespace backtesting
