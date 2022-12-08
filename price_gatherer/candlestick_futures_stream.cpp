#include "candlestick_futures_stream.hpp"
#include <iostream>

namespace binance {

void candlestick_futures_stream_t::onResultAvailable(candlestick_data_t data) {
  auto &os = m_tradeMap.dataMap[data.tokenName];
  os << data.eventTime << "," << data.startTime << "," << data.closeTime << ","
     << data.interval << "," << data.firstTradeID << "," << data.lastTradeID
     << "," << data.openPrice << "," << data.closePrice << "," << data.highPrice
     << "," << data.lowPrice << "," << data.baseAssetVolume << ","
     << data.numberOfTrades << "," << data.klineIsClosed << ","
     << data.quoteAssetVolume << "," << data.tbBaseAssetVolume << ","
     << data.tbQuoteAssetVolume << "\n";
  if (++m_flushInterval == 10'000) {
    m_flushInterval = 0;
    os.flush();
  }
}

void candlestick_futures_stream_t::writeCSVHeader() {

  for (auto &[_, file] : m_tradeMap.dataMap) {
    if (file.isOpen()) {
      file << "EventTime,StartTime,CloseTime,Interval,FirstTradeID,"
              "LastTradeID,OpenPrice,ClosePrice,HighPrice,LowPrice,"
              "BaseAssetVol,NumberOfTrades,IsKlineClosed,QuoteAssetVol,"
              "TakerBuyBAV,TakerBuyQAV" // base/quote asset volume
           << std::endl;
    }
  }
}

} // namespace binance
