import jbacktest as jb
import datetime as dt
import multiprocessing, time, os
from threading import Thread


def klineCallback(klineData):
    print(klineData)


def bookTickerCallback(btData):
    print(btData)


def onNewTrades(trades):
    print(trades)


def onNewDepthData(depth):
    pass
    # print(f"BTCUSDT(Futures) -> {jb.getCurrentPrice('BTCUSDT', jb.TradeType.futures)}")
    # print(f"BTCUSDT(Spot) -> {jb.getCurrentPrice('BTCUSDT', jb.TradeType.spot)}")
    # print('')


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
    os.chdir(os.path.dirname(__file__))
    appConfig = jb.AppConfig()
    appConfig.symbols = ["BTCUSDT", "ETHUSDT"]
    appConfig.trades = ["FUTURES", "SPOT"]
    appConfig.path = f'{os.getcwd()}/backtestingFiles'
    appConfig.dateStart = tenDaysAgo.strftime('%Y-%m-%d %H:%M:%S')
    appConfig.dateEnd = nowDt.strftime('%Y-%m-%d %H:%M:%S')

    backTest = jb.Backtesting.instance(appConfig)
    assert backTest is not None
    return backTest


def testSpotWallet():
    nowDt = dt.datetime.utcnow()
    nowTs = nowDt.timestamp()
    tenDaysAgo = nowDt - dt.timedelta(seconds=36000 * 24)

    assets = [jb.Asset("USDT", 1000), jb.Asset("ETH", 11), jb.Asset("BTC", 1)]
    for a in assets:
      print(f"{a.name}, {a.available}, {a.inUse}")

    newUser = jb.addUser(assets)
    assert newUser is not None
    
    print(len(newUser.orders)) # should be 0
    orderNumber = newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.2, jb.TradeSide.buy)
    print(orderNumber)
    assert(orderNumber == -1) # 16000 * 0.2 is $3'200 <- USDT balance is $1'000

    orderNumber = newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.05, jb.TradeSide.buy)
    assert(orderNumber > 0)

    orderNumber = newUser.createSpotMarketOrder("ETCUSDT", 1.0, jb.TradeSide.buy)
    assert(orderNumber == -1 and (newUser.orders is not None))
    print(orderNumber)
    
    orders = newUser.orders
    print(len(orders))
    assert(len(orders) == 1)
    
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
    # partialFunc = partial(bookTickerCallback, newUser)
    btConfig.setCallback(onNewDepthData)
    
    jb.getContinuousKline(klineConfig)
    jb.getContinuousBookTicker(btConfig)
    jb.registerTradesCallback(spotTrade, onNewTrades)
    jb.registerDepthCallback(spotTrade, onNewDepthData)
    
    testDiscreteKline(nowDt, nowTs, tenDaysAgo)
    testDiscreteBookticker(nowDt, nowTs, tenDaysAgo)
    testSpotMarketOrder()
    testSpotLimitOrder()


def testFuturesWallet():
    assets = [jb.Asset("USDT", 5000), jb.Asset("BNB", 10)]
    for a in assets:
      print(f"FuturesAsset: {a.name}, {a.available}, {a.inUse}")

    newUser = jb.addUser(assets)
    assert newUser is not None
    
    futures = jb.TradeType.futures
    
    newUser.leverage = 5
    assert(newUser.leverage == 5)
    
    newUser.leverage = 0.1
    assert(newUser.leverage == 1)
    
    newUser.leverage = 125
    assert(newUser.leverage == 125)
    
    newUser.leverage = 126
    assert(newUser.leverage == 125)
    
    newUser.leverage = 5
    assert(newUser.leverage == 5)

    assert(len(newUser.orders) == 0) # should be 0
    btcusdtPrice = 22000

    # buy 0.2 worth of BTCUSDT at price 22'000 at leverage 5 -> should cost (22'000 * 0.2) / 5 == 880
    orderNumber = newUser.createFuturesLimitOrder("BTCUSDT", btcusdtPrice, 0.2, jb.TradeSide.long)
    assert(orderNumber > 1)
    assert(len(newUser.orders) == 1)
    
    ## balance should be 5000 - 880 == $4120
    assert(newUser.assets[0].available == 4120)
    
    ## after this order, balance should be $4120 - $210 == $3910
    orderNumber = newUser.createFuturesLimitOrder("BTCUSDT", btcusdtPrice - 1000, 0.05, jb.TradeSide.long)
    assert(orderNumber > 1) # successful
    assert(len(newUser.orders) == 2)
    assert(newUser.assets[0].available == 3910)
    
    ## after this order, balance should be $3910 - $50 == $3860
    orderNumber = newUser.createFuturesMarketOrder("ETHUSDT", 50, jb.TradeSide.short)
    assert(orderNumber > 1)
    assert(newUser.assets[0].available == 3860)
    assert(len(newUser.orders) == 3)
    print(f"ETH Order number = {orderNumber}")
    for pos in newUser.positions:
      print (f"EntryPrice(ETH): {pos.entryPrice}, Qty: {pos.size}, Leverage: {pos.leverage}, LiqPrice: {pos.liquidationPrice}")
      
    ## after this order, balance should be $3860 - $3000 == $860
    orderNumber = newUser.createFuturesMarketOrder("BTCUSDT", 3000, jb.TradeSide.long)
    assert(orderNumber > 1)
    assert(newUser.assets[0].available == 860)
    assert(len(newUser.orders) == 4)
    assert(len(newUser.positions) == 2)
    print(f"BTC Order number = {orderNumber}")
    
    pos = newUser.positions[1]
    print (f"EntryPrice(BTC): {pos.entryPrice}, Qty: {pos.size}, Leverage: {pos.leverage}, LiqPrice: {pos.liquidationPrice}")

    orderNumber = newUser.createFuturesMarketOrder("ETHUSDT", 600, jb.TradeSide.long)
    # pos 0 is not BTCUSDT as the initial position has been closed and a new one reopened
    assert(orderNumber > 1)
    assert(len(newUser.orders) == 5)
    assert(len(newUser.positions) == 2)

    pos = newUser.positions[1]
    print (f"EntryPrice: {pos.entryPrice}, Qty: {pos.size}, Leverage: {pos.leverage}, LiqPrice: {pos.liquidationPrice}")


def main():
    nowDt = dt.datetime.utcnow()
    nowTs = nowDt.timestamp()
    tenDaysAgo = nowDt - dt.timedelta(seconds=36000 * 24)
    
    backTest = createBacktestObject(nowDt, nowTs, tenDaysAgo)
    backTest.run()

    # allow some time for the backtest object to call run()
    time.sleep(2)
    
    # spotThread = Thread(target=testSpotWallet)
    futuresThread = Thread(target=testFuturesWallet)
    bgThread = Thread(target=spinPrint)
    
    # spotThread.start()
    futuresThread.start()
    bgThread.start()
    
    # spotThread.join()
    futuresThread.join()
    bgThread.join()
    print ("End of test")


if __name__ == "__main__":
    main()
