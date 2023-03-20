#pragma once

#include <functional>

namespace backtesting {
struct indicator_data_t {
  //
};

using indicator_callback_t = std::function<void(indicator_data_t)>;
} // namespace backtesting
