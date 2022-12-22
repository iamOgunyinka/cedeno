#include "candlestick_spot_stream.hpp"
#include <iostream>

namespace binance {

void candlestick_spot_stream_t::onResultAvailable(
    candlestick_data_t const &data) {
  auto &os = m_tradeMap.dataMap[data.tokenName];
  if (os.rewriteHeader())
    writeCSVHeader(os);

  os.write(data.eventTime, data.startTime, data.closeTime, data.interval,
           data.firstTradeID, data.lastTradeID, data.openPrice, data.closePrice,
           data.highPrice, data.lowPrice, data.baseAssetVolume,
           data.numberOfTrades, data.klineIsClosed, data.quoteAssetVolume,
           data.tbBaseAssetVolume, data.tbQuoteAssetVolume);
}

void candlestick_spot_stream_t::writeCSVHeader(binance::locked_file_t &os) {
  if (os.isOpen()) {
    os.write("EventTime,StartTime,CloseTime,Interval,FirstTradeID,"
             "LastTradeID,OpenPrice,ClosePrice,HighPrice,LowPrice,"
             "BaseAssetVol,NumberOfTrades,IsKlineClosed,QuoteAssetVol,"
             "TakerBuyBAV,TakerBuyQAV"); // base/quote asset volume
    os.rewriteHeader(false);
  }
}
} // namespace binance
