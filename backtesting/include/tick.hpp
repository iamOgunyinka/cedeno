#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <map>
#include <vector>

#include "indicator_data.hpp"

namespace net = boost::asio;

namespace backtesting {

class tick_t {
public:
  void setCallback(indicator_callback_t func) { m_callback = func; }
  static tick_t &tickInstance(net::io_context &ioContext,
                              std::vector<size_t> const &ticks);

private:
  tick_t(net::io_context &ioContext, std::vector<size_t> const &ticks);
  void onTimerTimedout(size_t const, net::deadline_timer *);

  net::io_context &m_ioContext;
  std::vector<std::unique_ptr<net::deadline_timer>> m_timers;
  std::map<net::deadline_timer *, indicator_data_t> m_indicatorMap;
  indicator_callback_t m_callback = nullptr;
};

} // namespace backtesting
