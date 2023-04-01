import jbacktest as jb
from threading import Thread
import time

some = 5000

def busySpin():
  while jb.isRunning():
    time.sleep(0.5)
  print ("Done")


def onTick(data):
  global some
  
  if data['5'].isClosed:
    print (f"{data['5']['BTCUSDT'][-1].ema.price}")

  some -= 1
  if (some <= 0):
    jb.endSimulation() # will call onEnd


def onStart():
  print ("Starting")


def onEnd():
  print ("Ended")
  pass


if __name__ == '__main__':
  jb.startSimulation(onTick=onTick, onEnd=onEnd, onStart=onStart)

  spin = Thread(target=busySpin)
  spin.start()
  spin.join()
