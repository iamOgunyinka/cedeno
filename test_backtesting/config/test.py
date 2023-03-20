import jbacktest as jb

def onTick(data):
  # program logic
  pass


def onEnd():
  # jb.showProfit()
  pass


if __name__ == '__main__':
  if not jb.loadConfigFile('./config/config.ini'):
    raise Exception("Unable to read configuration file")
  jb.onTick = onTick
  jb.onCompletion = onEnd
  jb.start()
