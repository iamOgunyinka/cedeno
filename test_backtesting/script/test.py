import jbacktest as jb
import datetime as dt
import multiprocessing, time
from multiprocessing import Process


def klineCallback(klineData):
    print(klineData)


def bookTickerCallback(btData):
    print(btData)


def onNewTrades(trades):
    print(trades)


def onNewDepthData(depth):
    print(depth)


def spinPrint():
    while True:
        time.sleep(1)


def launchBacktest(backTest):
    print("launchBacktest")
    backTest.run()


def testDiscreteKline(nowDt, nowTs, tenDaysAgo):
    print("testDiscreteKline")
    klineConfig = jb.KlineConfig()
    klineConfig.tradeType = jb.TradeType.futures
    klineConfig.symbol = "BTCUSDT"
    klineConfig.interval = jb.DataInterval.one_minute
    klineConfig.startTime = int(tenDaysAgo.timestamp())
    klineConfig.endTime = int(nowTs)
    data = jb.getDiscreteKline(klineConfig)
    print(data)


def testDiscreteBookticker(nowDt, nowTs, tenDaysAgo):
    print("testDiscreteBookticker")
    btConfig = jb.BooktickerConfig()
    btConfig.symbols = ["BTCUSDT"]
    btConfig.tradeType = jb.TradeType.spot
    btConfig.startTime = int(tenDaysAgo.timestamp())
    btConfig.endTime = int(nowTs)
    d = jb.getBookticker(btConfig)


def testSpotMarketOrder():
    print("testSpotMarketOrder")
    pass

    
def testSpotLimitOrder():
    print("testSpotLimitOrder")
    pass
    

def createBacktestObject(nowDt, nowTs, tenDaysAgo):
    appConfig = jb.AppConfig()
    appConfig.symbols = ["BTCUSDT"]
    appConfig.trades = ["SPOT"]
    appConfig.path = "/home/joshuaogunyinka/Downloads/JoJo/cedeno/bin/backtestingFiles"
    appConfig.dateStart = tenDaysAgo.strftime('%Y-%m-%d %H:%M:%S')
    appConfig.dateEnd = nowDt.strftime('%Y-%m-%d %H:%M:%S')

    # appConfig.klineConfig = klineConfig
    # appConfig.booktickerConfig = btConfig

    backTest = jb.Backtesting.instance(appConfig)
    assert backTest is not None
    return backTest


def main():
    nowDt = dt.datetime.utcnow()
    nowTs = nowDt.timestamp()
    tenDaysAgo = nowDt - dt.timedelta(seconds=36000 * 24)
    
    backTest = createBacktestObject(nowDt, nowTs, tenDaysAgo)
    backTest.run()

    # allow some time for the backtest object to call run()
    time.sleep(2)
    assets = [jb.SpotWalletAsset("USDT", 1000), jb.SpotWalletAsset("ETH", 11), jb.SpotWalletAsset("BTC", 1)]
    for a in assets:
      print(f"{a.symbolName}, {a.available}, {a.inUse}")

    newUser = jb.addUser(assets)
    assert newUser is not None
    
    print(len(newUser.orders)) # should be 0
    orderNumber = newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.2, jb.TradeSide.buy)
    print(orderNumber)
    assert(orderNumber == -1) # 16000 * 0.2 is $3'200 <- USDT balance is $1'000

    orderNumber = newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.05, jb.TradeSide.buy)
    assert(orderNumber > 0)

    orderNumber = newUser.createSpotMarketOrder("ETHUSDT", 1.0, jb.TradeSide.buy)
    assert(orderNumber == -1 and (newUser.orders is not None))
    print(orderNumber)
    
    orders = newUser.orders
    print(len(orders))
    assert(len(orders) == 2)
    
    spotTrade = jb.TradeType.spot
    klineConfig = jb.KlineConfig()
    klineConfig.tradeType = spotTrade
    klineConfig.symbol = "BTCUSDT"
    klineConfig.interval = jb.DataInterval.one_second
    klineConfig.startTime = int(tenDaysAgo.timestamp())
    klineConfig.endTime = int(nowTs)
    klineConfig.setCallback(klineCallback)

    btConfig = jb.BooktickerConfig()
    btConfig.symbols = ["BTCUSDT"]
    btConfig.tradeType = jb.TradeType.futures
    btConfig.startTime = int(tenDaysAgo.timestamp())
    btConfig.endTime = int(nowTs)
    btConfig.setCallback(bookTickerCallback)
    
    jb.getContinuousKline(klineConfig)
    jb.getContinuousBookTicker(btConfig)
    jb.registerTradesCallback(spotTrade, onNewTrades)
    jb.registerDepthCallback(spotTrade, onNewDepthData)
    
    testDiscreteKline(nowDt, nowTs, tenDaysAgo)
    testDiscreteBookticker(nowDt, nowTs, tenDaysAgo)
    testSpotMarketOrder()
    testSpotLimitOrder()
    
    p = Process(target=spinPrint)
    p.start()
    p.join()
    print ("End of test")


if __name__ == "__main__":
    main()
