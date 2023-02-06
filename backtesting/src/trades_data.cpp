#include "trades_data.hpp"
#include "container.hpp"

namespace backtesting {

trades_callback_map_t recentTradesCallbacks{};
utils::waitable_container_t<trade_list_t> allNewTradeList{};

} // namespace backtesting
