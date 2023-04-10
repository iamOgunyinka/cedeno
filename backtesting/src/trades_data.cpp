#include "trades_data.hpp"
#include "container.hpp"

namespace backtesting {

trades_callback_map_t recentTradesCallbacks{};
utils::waitable_container_t<trade_list_t> allNewTradeList{};

exchange_trade_data_t exchange_trade_data_t::dataFromCSVStream(
    data_streamer_t<exchange_trade_data_t> &dataStreamer) {
  exchange_trade_data_t data;
  try {
    auto row = dataStreamer.getNextRow();
    if (row.empty())
      return exchange_trade_data_t{};
    data = tradeFromCSVRow(row);
  } catch (std::exception const &) {
  }

  return data;
}

exchange_trade_data_t
exchange_trade_data_t::tradeFromCSVRow(csv::CSVRow const &row) {

  if (!isExpectedRowCount(row.size()))
    throw std::runtime_error("unexpected columns in a row, expects 8");

  auto iter = row.begin();
  exchange_trade_data_t data{};
  data.eventTimeMs = getNumber<uint64_t>(iter++);
  data.aggregateTradeID = getNumber<uint64_t>(iter++);
  data.firstTradeID = getNumber<uint64_t>(iter++);
  data.lastTradeID = getNumber<uint64_t>(iter++);
  data.tradeTimeMs = getNumber<uint64_t>(iter++);
  data.price = getNumber<double>(iter++);
  data.quantity = getNumber<double>(iter++);
  data.isBuyerMarketMaker = static_cast<bool>(getNumber<int>(iter));

  return data;
}

exchange_trade_data_t localToExchangeTrade(trade_data_t const &tradeData,
                                           bool const isMarketMaker) {
  exchange_trade_data_t d;
  d.price = tradeData.amountPerPiece;
  d.eventTimeMs = d.tradeTimeMs = (tradeData.eventTime * 1'000);
  d.quantity = tradeData.quantityExecuted;
  d.aggregateTradeID = tradeData.tradeID;
  d.isBuyerMarketMaker = static_cast<int>(isMarketMaker);
  d.firstTradeID = d.firstTradeID = d.aggregateTradeID;
  return d;
}
} // namespace backtesting
