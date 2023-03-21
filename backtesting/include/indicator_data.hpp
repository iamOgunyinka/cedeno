#pragma once

#include <functional>

namespace backtesting {
struct indicator_data_t {
  time_t time = 0;
};

using indicator_callback_t = std::function<void(indicator_data_t)>;
} // namespace backtesting
