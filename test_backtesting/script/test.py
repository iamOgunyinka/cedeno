import jbacktest as jb
import datetime as dt
import multiprocessing, time, os, unittest
from threading import Thread


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


def launchBacktest(self, backTest):
  backTest.run()

def spinPrint(self):
    while True:
        time.sleep(1)


class SpotTest(unittest.TestCase):
  def spotMarketOrderTest(self):
    pass

  def bookTickerCallback(self, btData):
    print(btData)
    
  def onNewTrades(self, trades):
    print(trades)

  def spotLimitOrderTest(self):
    print("spotLimitOrderTest")
    pass

  def discreteKlineCB(self, nowDt, nowTs, tenDaysAgo):
    klineConfig = jb.KlineConfig()
    klineConfig.tradeType = jb.TradeType.futures
    klineConfig.symbol = "BTCUSDT"
    klineConfig.interval = jb.DataInterval.one_minute
    klineConfig.startTime = int(tenDaysAgo.timestamp())
    klineConfig.endTime = int(nowTs)
    data = jb.getDiscreteKline(klineConfig)

  def discreteBooktickerCB(self, nowDt, nowTs, tenDaysAgo):
    btConfig = jb.BooktickerConfig()
    btConfig.symbols = ["BTCUSDT"]
    btConfig.tradeType = jb.TradeType.spot
    btConfig.startTime = int(tenDaysAgo.timestamp())
    btConfig.endTime = int(nowTs)
    d = jb.getBookticker(btConfig)  
    
  def klineCallback(self, klineData):
    pass
    # print(klineData)

  def onNewDepthData(self, depth):
    pass
    # print(f"BTCUSDT(Futures) -> {jb.getCurrentPrice('BTCUSDT', jb.TradeType.futures)}")
    # print(f"BTCUSDT(Spot) -> {jb.getCurrentPrice('BTCUSDT', jb.TradeType.spot)}")
    # print('')
    
  def test_spotWallet(self):
    nowDt = dt.datetime.utcnow()
    nowTs = nowDt.timestamp()
    tenDaysAgo = nowDt - dt.timedelta(seconds=36000 * 24)

    assets = [jb.Asset("USDT", 1000), jb.Asset("ETH", 11), jb.Asset("BTC", 1)]
    for a in assets:
      print(f"Asset: {a.name}, {a.available}, {a.inUse}")

    newUser = jb.addUser(assets)
    self.assertTrue(newUser is not None)
    self.assertTrue(len(newUser.orders) == 0)

    orderNumber = newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.2, jb.TradeSide.buy)
    # 16000 * 0.2 is $3'200 <- USDT balance is $1'000
    self.assertEqual(orderNumber, -1, "Order number should be -1")
    
    orderNumber = newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.05, jb.TradeSide.buy)
    self.assertTrue(orderNumber > 0)

    orderNumber = newUser.createSpotMarketOrder("ETCUSDT", 1.0, jb.TradeSide.buy)
    self.assertEqual(orderNumber, -1, "OrderNumber should be -1")
    self.assertTrue(newUser.orders is not None)

    orders = newUser.orders
    self.assertEqual(len(orders), 1, "Length of orders should be 1")
    
    spotTrade = jb.TradeType.spot
    klineConfig = jb.KlineConfig()
    klineConfig.tradeType = spotTrade
    klineConfig.symbol = "BTCUSDT"
    klineConfig.interval = jb.DataInterval.one_second
    klineConfig.startTime = int(tenDaysAgo.timestamp())
    klineConfig.endTime = int(nowTs)
    
    # func = partial()
    klineConfig.setCallback(self.klineCallback)

    btConfig = jb.BooktickerConfig()
    btConfig.symbols = ["BTCUSDT"]
    btConfig.tradeType = jb.TradeType.futures
    btConfig.startTime = int(tenDaysAgo.timestamp())
    btConfig.endTime = int(nowTs)
    # partialFunc = partial(bookTickerCallback, newUser)
    btConfig.setCallback(self.onNewDepthData)
    
    jb.getContinuousKline(klineConfig)
    jb.getContinuousBookTicker(btConfig)
    jb.registerTradesCallback(spotTrade, self.onNewTrades)
    jb.registerDepthCallback(spotTrade, self.onNewDepthData)
    
    self.discreteKlineCB(nowDt=nowDt, nowTs=nowTs, tenDaysAgo=tenDaysAgo)
    self.discreteBooktickerCB(nowDt=nowDt, nowTs=nowTs, tenDaysAgo=tenDaysAgo)
    self.spotMarketOrderTest()
    self.spotLimitOrderTest()


class FuturesTest(unittest.TestCase):
  def test_futuresWallet(self):
    assets = [jb.Asset("USDT", 5000), jb.Asset("BNB", 10)]
    for a in assets:
      print(f"FuturesAsset: {a.name}, {a.available}, {a.inUse}")

    newUser = jb.addUser(assets)
    self.assertTrue(newUser is not None)
    
    futures = jb.TradeType.futures
    
    newUser.leverage = 5
    self.assertEqual(newUser.leverage, 5, "Leverage should be 5")
    
    newUser.leverage = 0.1
    self.assertEqual(newUser.leverage, 1, "Leverage should be 1")
    
    newUser.leverage = 125
    self.assertEqual(newUser.leverage, 125, "Leverage should be 125")
    
    newUser.leverage = 126
    self.assertEqual(newUser.leverage, 125, "Leverage should be 125. It is clamped")
    
    newUser.leverage = 5
    self.assertEqual(newUser.leverage, 5, "Leverage should be 5")

    self.assertEqual(len(newUser.orders), 0, "Total orders should be 0")
    btcusdtPrice = 22000
    
    ## balance should be $5000
    self.assertTrue(newUser.assets[0].available == 5000)
    
    # buy 0.2 worth of BTCUSDT at price 22'000 at leverage 5 -> should cost (22'000 * 0.2) / 5 == 880
    orderNumber = newUser.createFuturesLimitOrder("BTCUSDT", btcusdtPrice, 0.2, jb.TradeSide.long)
    self.assertTrue(orderNumber > 1)
    self.assertTrue(len(newUser.orders) == 1)
    
    ## balance should be 5000 - 880 == $4120
    self.assertTrue(newUser.assets[0].available == 4120)
    
    ## after this order, balance should be $4120 - $210 == $3910
    orderNumber = newUser.createFuturesLimitOrder("BTCUSDT", btcusdtPrice - 1000, 0.05, jb.TradeSide.long)
    self.assertTrue(orderNumber > 1) # successful
    self.assertTrue(len(newUser.orders) == 2)
    self.assertTrue(newUser.assets[0].available == 3910)
    
    ## after this order, balance should be $3910 - $50 == $3860
    orderNumber = newUser.createFuturesMarketOrder("ETHUSDT", 50, jb.TradeSide.short)
    self.assertTrue(orderNumber > 1)
    self.assertTrue(newUser.assets[0].available == 3860)
    self.assertTrue(len(newUser.orders) == 3)
    print(f"ETH Order number = {orderNumber}")
    for pos in newUser.positions:
      print (f"EntryPrice(ETH): {pos.entryPrice}, Qty: {pos.size}, Leverage: {pos.leverage}, LiqPrice: {pos.liquidationPrice}")
      
    ## after this order, balance should be $3860 - $3000 == $860
    orderNumber = newUser.createFuturesMarketOrder("BTCUSDT", 3000, jb.TradeSide.long)
    self.assertTrue(orderNumber > 1)
    self.assertTrue(newUser.assets[0].available == 860)
    self.assertTrue(len(newUser.orders) == 4)
    self.assertTrue(len(newUser.positions) == 2)
    print(f"BTC Order number = {orderNumber}")
    
    pos = newUser.positions[1]
    print (f"EntryPrice(BTC): {pos.entryPrice}, Qty: {pos.size}, Leverage: {pos.leverage}, LiqPrice: {pos.liquidationPrice}")

    orderNumber = newUser.createFuturesMarketOrder("ETHUSDT", 600, jb.TradeSide.long)
    # pos 0 is not BTCUSDT as the initial position has been closed and a new one reopened
    pos = newUser.positions[1]
    self.assertTrue(orderNumber > 1)
    self.assertTrue(len(newUser.orders) == 5)
    self.assertTrue(len(newUser.positions) == 2)
    print (f"EntryPrice: {pos.entryPrice}, Qty: {pos.size}, Leverage: {pos.leverage}, LiqPrice: {pos.liquidationPrice}")
    
    print(f"Total open positions: {len(newUser.positions)}")
    print("Closing all positions")
    self.assertTrue(newUser.closeAllPositions())
    print(f"User's last balance is {newUser.assets[0].available}")
    

def runTests():
  nowDt = dt.datetime.utcnow()
  nowTs = nowDt.timestamp()
  tenDaysAgo = nowDt - dt.timedelta(seconds=36000 * 24)
    
  backTest = createBacktestObject(nowDt, nowTs, tenDaysAgo)
  backTest.run()
  # allow some time for the backtest object to call run()
  time.sleep(2)
  unittest.main()


def main():
  spin = Thread(target=spinPrint)
  spin.start()
  spin.join()
  print ("End of test")


if __name__ == '__main__':
  runTests()
  main()

