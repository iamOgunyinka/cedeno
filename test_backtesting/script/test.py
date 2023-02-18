import jbacktest as jb
from threading import Thread


def launchBacktest():
    appConfig = jb.AppConfig()
    appConfig.symbols = ["BTCUSDT"]
    appConfig.trades = ["SPOT"]
    appConfig.path = "/home/joshuaogunyinka/Downloads/JoJo/cedeno/bin/backtestingFiles"
    appConfig.dateStart = "2023-02-11 00:00:00"
    appConfig.dateEnd = "2023-02-14 00:00:00"

    backTest = jb.Backtesting.instance(appConfig)
    assert backTest is not None
    backTest.run()


def main():
    newThread = Thread(target=launchBacktest)
    newThread.start()
    launchBacktest()
    
    assets = [jb.SpotWalletAsset("USDT", 1000), jb.SpotWalletAsset("ETH", 11), jb.SpotWalletAsset("BTC", 1)]
    
    newUser = jb.addUser(assets)
    print(newUser)
    assert newUser is not None
    
    print(len(newUser.orders)) # should be 0
    assert(newUser.createSpotLimitOrder("BTCUSDT", 16000, 0.2, jb.TradeSide.buy))
    
    orderID = newUser.createSpotMarketOrder("ETHUSDT", 1.0, jb.TradeSide.buy)
    assert(orderID > 0)

    assert(len(newUser.orders) == 2)
    

if __name__ == "__main__":
    main()
