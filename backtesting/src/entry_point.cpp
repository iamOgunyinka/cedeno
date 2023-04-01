#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "entry_point.hpp"

#ifdef BT_USE_WITH_DB
#include "adaptor.hpp"
#include "database_connector.hpp"
#endif // BT_USE_WITH_DB

#include "global_data.hpp"
#include "tick.hpp"
#include <CLI11/CLI11.hpp>
#include <mini/ini.h>
#include <random>
#include <spdlog/spdlog.h>

#define PRINT_INFO(str, ...)                                                   \
  {                                                                            \
    if (verbose)                                                               \
      spdlog::info(str, ##__VA_ARGS__);                                        \
  }

#define ERROR_PARSE() (m_config.reset(), m_argumentParsed)
#define PRINT_ERROR(str, ...)                                                  \
  {                                                                            \
    if (verbose)                                                               \
      spdlog::error(str, ##__VA_ARGS__);                                       \
  }

#define ERROR_EXIT(str, ...)                                                   \
  PRINT_ERROR(str, ##__VA_ARGS__)                                              \
  return ERROR_PARSE();

using backtesting::utils::currentTimeToString;
using backtesting::utils::dateStringToTimeT;
using backtesting::utils::listContains;

bool verbose = false;
bool isRunning = false;

namespace backtesting {
#ifdef BT_USE_WITH_DB
void setupDummyList(token_data_list_t const &tokenList,
                    database_connector_t &dbConnector,
                    std::vector<db_user_t> &userList) {
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<> uid(0, tokenList.size());

  int result = dbConnector.addNewUser("joshua");
  assert(result > 0);
  userList = dbConnector.getUserIDs();
  assert(!userList.empty());
  assert(userList[0].userID == 1);

  db_user_t &user = userList[0];
  db_user_asset_list_t assets;
  assets.reserve(10);

  for (int i = 0; i < 10; ++i) {
    auto &token = tokenList[uid(gen)];
    db_user_asset_list_t::value_type userAsset;
    userAsset.ownerID = user.userID;
    userAsset.tokenName = token.name;
    userAsset.amountAvailable = 500.0;
    assets.push_back(std::move(userAsset));
  }

  db_user_order_list_t orderList;
  orderList.reserve(5);

  for (int i = 0; i < 5; ++i) {
    auto const isEven = i % 5 == 0;
    auto &token = tokenList[uid(gen)];
    db_user_order_t order;
    order.leverage = 10.0;
    order.market = int(isEven ? trade_market_e::limit : trade_market_e::market);
    order.priceLevel = double(i);
    order.quantity = double(i) + 1.0;
    order.side = int(isEven ? trade_side_e::buy : trade_side_e::sell);
    order.symbol = token.name;
    order.type = int(isEven ? trade_type_e::futures : trade_type_e::spot);
    order.userID = user.userID;
    orderList.push_back(std::move(order));
  }

  if (!dbConnector.addUserAssets(assets))
    throw std::runtime_error("assets should not be empty when testing");
  if (!dbConnector.addOrderList(orderList))
    throw std::runtime_error("user should have some orders for testing");
}

void saveTokenFromFileToDatabase(database_connector_t &dbConnector,
                                 db_token_list_t &dbTokenList,
                                 token_data_list_t &tokenList,
                                 std::string const &rootPath) {
  assert(dbTokenList.empty());

  std::vector<std::pair<trade_type_e, char const *>> const pair{
      {trade_type_e::futures, "futures.csv"}, {trade_type_e::spot, "spot.csv"}};

  auto const dirPath = std::filesystem::path(rootPath);
  if (!std::filesystem::exists(dirPath))
    return;

  for (auto const &[tradeType, filename] : pair) {
    auto const path = dirPath / filename;
    if (!std::filesystem::exists(path))
      continue;

    std::ifstream file(path, std::ios::in);
    if (!file)
      continue;

    std::string line;
    while (std::getline(file, line)) {
      utils::trim(line);
      if (!line.empty()) {
        auto const &splits = utils::splitString(line, ",");
        if (splits.size() != 3)
          continue;
        token_data_list_t::value_type d;
        d.name = splits[0];
        d.baseAsset = splits[1];
        d.quoteAsset = splits[2];
        d.tradeType = tradeType;

        db_token_list_t::value_type db;
        db.name = d.name;
        db.tradeType = (int)tradeType;
        dbTokenList.push_back(std::move(db));
        tokenList.push_back(std::move(d));
      }
    }
  }
  dbConnector.addTokenList(dbTokenList);
  dbTokenList.clear();
  dbTokenList = dbConnector.getListOfAllTokens();
}

#else
void readTokensFromFileImpl(token_data_list_t &result,
                            trade_type_e const tradeType,
                            std::string const &filename) {
  std::ifstream file(filename, std::ios::in);
  if (!file)
    return;

  std::string line;
  while (std::getline(file, line)) {
    utils::trim(line);
    if (!line.empty()) {
      auto const &splits = utils::splitString(line, ",");
      if (splits.size() != 3)
        continue;
      token_data_list_t::value_type d;
      d.name = utils::toUpperString(splits[0]);
      d.baseAsset = splits[1];
      d.quoteAsset = splits[2];
      d.tradeType = tradeType;
      result.push_back(std::move(d));
    }
  }
}

token_data_list_t readTokensFromFile(std::string const &rootPath) {
  std::vector<std::pair<trade_type_e, char const *>> const pair{
      {trade_type_e::futures, "futures.csv"}, {trade_type_e::spot, "spot.csv"}};

  auto const dirPath = std::filesystem::path(rootPath);
  backtesting::token_data_list_t result;

  if (!std::filesystem::exists(dirPath)) {
    PRINT_INFO("Path does not exist: {}", rootPath);
    return result;
  }

  for (auto const &[tradeType, filename] : pair) {
    auto const path = dirPath / filename;
    PRINT_INFO("Path to navigate: {}", path.string());

    if (!std::filesystem::exists(path)) {
      PRINT_INFO("Path does not exist");
      continue;
    }
    readTokensFromFileImpl(result, tradeType, path.string());
  }
  return result;
}
#endif
} // namespace backtesting

backtesting_t::backtesting_t()
    : m_config{std::nullopt}, m_argumentParsed(false),
      m_authenticatedData(false) {}

backtesting_t::backtesting_t(backtesting::configuration_t const &config)
    : backtesting_t{} {
  if (!parseImpl(config))
    throw std::logic_error("invalid configuration used");
}

bool backtesting_t::parseImpl(backtesting::configuration_t config) {
#ifdef BT_USE_WITH_DB
  if (config.dbLaunchType.empty())
    config.dbLaunchType = "development";

  if (config.dbConfigFilename.empty())
    config.dbConfigFilename = backtesting::getDatabaseConfigPath();

  auto const dbConfig = backtesting::parse_database_file(
      config.dbConfigFilename, config.dbLaunchType);
  if (!dbConfig) {
    spdlog::error("Unable to get database configuration values");
    return ERROR_PARSE();
  }
#endif

  verbose = config.verbose;
  if (config.streams.empty()) {
    PRINT_INFO("Streams not specified, will use 'DEPTH' as default")
    config.streams.push_back(DEPTH);
  } else {
    std::vector<std::string> const validStreams{TICKER, BTICKER, CANDLESTICK,
                                                DEPTH};
    for (auto &stream : config.streams) {
      if (!listContains(validStreams, stream)) {
        ERROR_EXIT("'{}' is not a valid stream type", stream)
      }
      for (auto &s : stream)
        s = tolower(s);
    }
  }

  if (config.tradeType.empty()) {
    PRINT_INFO("trade type not specified, will use 'SPOT' as default")
    config.tradeType = SPOT;
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    if (!listContains(validTrades, config.tradeType)) {
      ERROR_EXIT("'{}' is not a valid trade type", config.tradeType);
    }

    for (auto &t : config.tradeType)
      t = tolower(t);
  }

  if (config.tokenList.empty()) {
    PRINT_INFO("token list is empty, using 'BTCUSDT' as the default");
    config.tokenList.emplace_back("BTCUSDT");
#ifdef _DEBUG
    PRINT_INFO("adding 'ETHUSDT' to the token list");
    config.tokenList.emplace_back("ETHUSDT");
#endif // _DEBUG
  } else {
    if (config.tokenList.size() > 4) {
      ERROR_EXIT("The maximum allowed symbols is 4");
    }

    for (auto &token : config.tokenList) {
      for (auto &t : token)
        t = toupper(t);
    }
  }

#ifdef BT_USE_WITH_INDICATORS
  if (!indicators::isValidIndicatorConfiguration(config.indicatorConfig)) {
    ERROR_EXIT("invalid configuration set for the indicator");
  }
#endif

  if (config.rootDir.empty()) {
    config.rootDir =
        (std::filesystem::current_path() / "backtestingFiles").string();
  }

  PRINT_INFO("Root directory: {}", config.rootDir);
  if (!std::filesystem::exists(config.rootDir)) {
    ERROR_EXIT("'{}' does not exist.", config.rootDir);
  }

  if (config.dateFromStr.empty()) {
    constexpr std::size_t const last24hrs = 3'600 * 24;
    config.dateFromStr = fmt::format(
        "{} 00:00:00",
        currentTimeToString(std::time(nullptr) - last24hrs, "-").value());
    PRINT_INFO("Start date not specified, will use '{}'", config.dateFromStr)
  }

  if (config.dateToStr.empty()) {
    config.dateToStr = fmt::format(
        "{} 23:59:59", currentTimeToString(std::time(nullptr), "-").value());
    PRINT_INFO("End-date not specified, will use '{}'", config.dateToStr)
  }

  if (config.klineConfig) {
    using backtesting::getContinuousKlineData;

    auto &klineConfig = *config.klineConfig;
    if (!(klineConfig.callback &&
          getContinuousKlineData(std::move(klineConfig)))) {
      ERROR_EXIT("There was a problem setting the kline config");
    }
  }

  if (config.bookTickerConfig) {
    using backtesting::getContinuousBTickerData;

    auto &bookTickerConfig = *config.bookTickerConfig;
    if (!bookTickerConfig.callback &&
        getContinuousBTickerData(std::move(bookTickerConfig))) {
      ERROR_EXIT("There was a problem setting the book ticker config");
    }
  }
  auto &globalRtData = global_data_t::instance();
  globalRtData.rootPath = config.rootDir;

  if (auto const optStartTime = dateStringToTimeT(config.dateFromStr);
      optStartTime.has_value()) {
    globalRtData.startTime = *optStartTime;
  } else {
    ERROR_EXIT("Unable to calculate the start date from user input");
  }

  if (auto const optEndTime = dateStringToTimeT(config.dateToStr);
      optEndTime.has_value()) {
    globalRtData.endTime = *optEndTime;
  } else {
    ERROR_EXIT("Unable to calculate the end date from user input");
  }

  if (globalRtData.startTime > globalRtData.endTime)
    std::swap(globalRtData.startTime, globalRtData.endTime);

  globalRtData.listOfFiles = backtesting::utils::getListOfCSVFiles(
      config.tokenList, config.tradeType, config.streams,
      globalRtData.startTime, globalRtData.endTime, config.rootDir);

  if (globalRtData.listOfFiles.empty()) {
    ERROR_EXIT("No files found matching the user-defined criteria");
  }

#ifdef BT_USE_WITH_DB
  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  databaseConnector->username(dbConfig.username);
  databaseConnector->password(dbConfig.password);
  databaseConnector->database_name(dbConfig.db_dns);
  if (!databaseConnector->connect())
    return ERROR_PARSE();
#endif

  globalRtData.futuresMakerFee = std::clamp(config.futuresMakerFee, 0.0, 0.02);
  globalRtData.futuresTakerFee =
      std::clamp(config.futuresTakerFee, 0.017, 0.04);
  globalRtData.spotMakerFee = std::clamp(config.spotMakerFee, 0.02, 0.1);
  globalRtData.spotTakerFee = std::clamp(config.spotTakerFee, 0.04, 0.1);

  PRINT_INFO("start date: {}", config.dateFromStr);
  PRINT_INFO("end date: {}", config.dateToStr);
  PRINT_INFO("rootDir: {}", config.rootDir);

  for (auto const &token : config.tokenList)
    PRINT_INFO("token: {}", token);
  for (auto const &s : config.streams)
    PRINT_INFO("stream: {}", s);
  PRINT_INFO("trade: {}", config.tradeType);

#ifdef BT_USE_WITH_INDICATORS
  globalRtData.indicatorConfig = std::move(config.indicatorConfig);
  globalRtData.ticks = config.ticks;
#endif

  m_config.emplace(config);
  m_argumentParsed = true;
  return m_argumentParsed;
}

void backtesting_t::reset() {
  m_config.reset();
  m_argumentParsed = false;
  m_authenticatedData = false;
}

bool backtesting_t::parse(int argc, char **argv) {
  m_argumentParsed = false;

  CLI::App app{"backtesting software"};
  auto &args = m_config.emplace();

#ifdef BT_USE_WITH_DB
  app.add_option("--db-config", args.dbConfigFilename,
                 fmt::format("database configuration filename "
                             "(default: '{}')",
                             args.dbConfigFilename));
  app.add_option("--db-launch-type", args.dbLaunchType,
                 fmt::format("database configuration"
                             " launch type(default: '{}')",
                             args.dbLaunchType));
#endif

  app.add_option("--tokens", args.tokenList,
                 "a list of token pairs [e.g. btcusdt(default), ethusdt]");
  app.add_option("--streams", args.streams,
                 "A list of the streams(s) to run. Valid options are: "
                 "[trade, ticker, bookticker, depth(default), kline]");
  app.add_option("--trade-type", args.tradeType,
                 "The trade type. Valid options are: "
                 "[futures OR spot(default)]");
  app.add_option("--start-date", args.dateFromStr,
                 "the start datetime (e.g. 2022-12-01 00:00:00)");
  app.add_option("--end-date", args.dateToStr,
                 "the end datetime (e.g. 2022-12-31 23:59:50)");
  app.add_option("--futures-mf", args.futuresMakerFee,
                 "futures maker fee (default: 0.02%)");
  app.add_option("--futures-tf", args.futuresTakerFee,
                 "futures taker fee (default: 0.04%)");
  app.add_option("--spot-mf", args.spotMakerFee,
                 "spots maker fee(default: 0.1%)");
  app.add_option("--spot-tf", args.spotTakerFee,
                 "spots taker fee (default: 0.1%)");
  app.add_option(
      "--root-dir", args.rootDir,
      "Root directory where the historical data are stored (default: `pwd`)");
  app.add_option("-v,--verbose", verbose,
                 "print out every step of the process(default: false)");

  try {
    app.parse(argc, argv);
  } catch (CLI::ParseError &e) {
    if (e.get_exit_code() != 0)
      PRINT_ERROR(e.what());
    app.exit(e);
    return ERROR_PARSE();
  }

  return parseImpl(args);
}

bool backtesting_t::prepareData() {
  if (!m_argumentParsed) {
    ERROR_EXIT(
        "The user-settings has not been parsed yet, call .parse on the object");
  }

  auto &globalRtData = global_data_t::instance();

#ifdef BT_USE_WITH_DB
  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  auto dbTokenList = databaseConnector->getListOfAllTokens();
  if (dbTokenList.empty()) {
    saveTokenFromFileToDatabase(*databaseConnector, dbTokenList,
                                globalRtData.allTokens, m_config->rootDir);
  }

  auto dbUserList = databaseConnector->getUserIDs();
  if (dbUserList.empty())
    setupDummyList(globalRtData.allTokens, *databaseConnector, dbUserList);

  auto &users = globalRtData.allUserAccounts;
  users.clear();
  users.reserve(dbUserList.size());

  for (auto const &u : dbUserList) {
    auto user = std::make_shared<backtesting::user_data_t>();
    user->assets = backtesting::adaptor::dbUserAssetsToBtUserAssets(
        databaseConnector->getAllAssetsByUser(u.userID));
    user->orders = backtesting::adaptor::dbOrderListToBtOrderList(
        databaseConnector->getOrderForUser(u.userID), globalRtData.allTokens,
        user.get());
    user->trades = backtesting::adaptor::dbTradeListToBtTradeList(
        databaseConnector->getTradesForUser(u.userID), globalRtData.allTokens);
    users.push_back(std::move(user));
  }

#else
  globalRtData.allTokens = backtesting::readTokensFromFile(m_config->rootDir);
  PRINT_INFO("Size of allTokens: {}", globalRtData.allTokens.size());
#endif

  std::sort(globalRtData.allTokens.begin(), globalRtData.allTokens.end(),
            [](backtesting::token_data_list_t::value_type const &a,
               backtesting::token_data_list_t::value_type const &b) {
              return std::tie(a.name, a.tradeType) <
                     std::tie(b.name, b.tradeType);
            });
  using backtesting::utils::toUpperString;
  using backtesting::utils::trim;
  for (auto &token : globalRtData.allTokens) {
    trim(token.name);
    trim(token.quoteAsset);
    trim(token.baseAsset);
    if (!token.quoteAsset.empty())
      globalRtData.validSymbols.insert(toUpperString(token.quoteAsset));

    if (!token.baseAsset.empty())
      globalRtData.validSymbols.insert(toUpperString(token.baseAsset));
  }

  m_authenticatedData = true;
  return true;
}

backtesting_t *newBTInstance(backtesting::configuration_t const &config) {
  auto bt = backtesting::getGlobalBTInstance();
  bt->reset();

  if (!(bt->parseImpl(config) && bt->prepareData() && bt->isReady()))
    return nullptr;
  return bt;
}

namespace backtesting {

user_data_t *getGlobalUser() {
  auto &users = global_data_t::instance().allUserAccounts;
  for (size_t i = 0; i < users.size(); ++i) {
    if (users[i]->m_isGlobalUser)
      return users[i].get();
  }
  return nullptr;
}

std::unique_ptr<backtesting_t> &getGlobalBTInstanceImpl() {
  static std::unique_ptr<backtesting_t> bt = nullptr;
  if (!bt)
    bt = std::make_unique<backtesting_t>();
  return bt;
}

backtesting_t *getGlobalBTInstance() { return getGlobalBTInstanceImpl().get(); }

bool createBTInstanceFromConfigFile(std::string const &filename) {
  mINI::INIFile file(filename);
  mINI::INIStructure iniStruct{};

  if (!file.read(iniStruct))
    return false;

  configuration_t config;
  double leverage = 1.0;
  backtesting::wallet_asset_list_t assets;
  bool verbosity = false;

  for (auto const &[key, value] : iniStruct) {
    auto const keyName = utils::trim_copy(utils::toUpperString(key));
    if (keyName.compare("APP") == 0) {
      auto const verbosityStr = utils::trim_copy(value.get("verbose"));
      if (verbosityStr.empty())
        return false;
      verbosity = static_cast<bool>(std::clamp(std::stoi(verbosityStr), 0, 1));
    } else if (keyName.compare("ASSETS") == 0) {
      for (auto const &[asset, balance] : value) {
        auto const assetName = utils::trim_copy(utils::toUpperString(asset));
        double const v = std::stod(utils::trim_copy(balance));
        assets.push_back(backtesting::wallet_asset_t{assetName, v});
      }
    } else if (keyName.compare("TRADES") == 0) {
      for (auto const &[n, v] : value) {
        auto const name = utils::trim_copy(utils::toUpperString(n));
        if (name.compare("SYMBOLS") == 0) {
          config.tokenList = utils::splitString(utils::trim_copy(v), ",");
        } else if (name.compare("TYPE") == 0) {
          config.tradeType = utils::trim_copy(v);
        } else if (name.compare("PATH") == 0) {
          config.rootDir = utils::trim_copy(v);
          utils::removeAllQuotes(config.rootDir);
        } else if (name.compare("STARTDATE") == 0) {
          config.dateFromStr = utils::trim_copy(v);
          utils::removeAllQuotes(config.dateFromStr);
        } else if (name.compare("ENDDATE") == 0) {
          config.dateToStr = utils::trim_copy(v);
          utils::removeAllQuotes(config.dateToStr);
        } else if (name.compare("LEVERAGE") == 0) {
          leverage = std::stod(utils::trim_copy(v));
        }
      }
    } else if (keyName.compare("FEES") == 0) {
      for (auto const &[variable_, amount_] : value) {
        auto const variable = utils::toUpperString(utils::trim_copy(variable_));
        double const amount = std::stod(utils::trim_copy(amount_));
        if (variable == "FUTURESMAKER")
          config.futuresMakerFee = amount;
        else if (variable == "FUTURESTAKER")
          config.futuresTakerFee = amount;
        else if (variable == "SPOTMAKER")
          config.spotMakerFee = amount;
        else if (variable == "SPOTTAKER")
          config.spotTakerFee = amount;
      }
    }
#ifdef BT_USE_WITH_INDICATORS
    else if (utils::startsWith(keyName, "INDICATORS")) {
      auto const indicatorSplit = utils::splitString(keyName, ".");
      if (indicatorSplit.size() == 1) { // whole string match
        for (auto const &[name_, val] : value) {
          auto const name = utils::trim_copy(utils::toUpperString(name_));
          if (name == "TICK") {
            auto &ticks = config.ticks;
            for (auto const &tick : utils::splitString(val, ","))
              ticks.push_back(std::stod(utils::trim_copy(tick)));

            // sort and remove possible duplicates
            std::sort(ticks.begin(), ticks.end(), std::less<size_t>{});
            ticks.erase(std::unique(ticks.begin(), ticks.end()), ticks.end());
          }
        }
      } else if (indicatorSplit.size() == 2) {
        auto const indicatorName =
            utils::toUpperString(utils::trim_copy(indicatorSplit[1]));
        if (indicatorName == "QTY_IN") {
          //
        } else if (indicatorName == "QTY_OUT") {
          //
        } else if (indicatorName == "AVG_OUT") {
          //
        } else if (indicatorName == "AVG_IN") {
          //
        } else if (indicatorName == "TICK_IN") {
          //
        } else if (indicatorName == "TICK_OUT") {
          //
        } else if (indicatorName == "BUY_VS_SELL") {
          //
        } else if (indicatorName == "MODE") {
          //
        } else if (indicatorName == "EMA") {
          auto const nValue = value.get("n");
          if (nValue.empty())
            return false;
          config.indicatorConfig.push_back({{"ema"}, {"n:" + nValue}});
        } else if (indicatorName == "SMA") {
          auto const nValue = value.get("n");
          if (nValue.empty())
            return false;
          config.indicatorConfig.push_back({{"sma"}, {"n:" + nValue}});
        } else if (indicatorName == "MACD") {
          config.indicatorConfig.push_back({"macd"});
        } else if (indicatorName == "WMA") {
          auto const nValue = utils::trim_copy(value.get("n"));
          auto const wValue = utils::trim_copy(value.get("w"));
          if (nValue.empty() || wValue.empty())
            return false;
          config.indicatorConfig.push_back(
              {{"wma"}, {"n:" + nValue}, {"w:" + wValue}});
        } else if (indicatorName == "ATR") {
          auto const nValue = value.get("n");
          if (nValue.empty())
            return false;
          config.indicatorConfig.push_back({{"atr"}, {"n:" + nValue}});
        } else if (indicatorName == "SAR") {
          auto const aValue = utils::trim_copy(value.get("a"));
          auto const emaValue = utils::trim_copy(value.get("ema"));
          if (aValue.empty() || emaValue.empty())
            return false;
          config.indicatorConfig.push_back(
              {{"sar"}, {"a:" + aValue}, {"ema:" + emaValue}});
        }
      }
    }
#endif
  }

  config.verbose = verbosity;
  auto bt = ::newBTInstance(std::move(config));
  if (bt == nullptr)
    return false;

  if (auto const userID = global_data_t::newUser(std::move(assets));
      userID < 0) {
    return false;
  } else {
    auto &users = global_data_t::instance().allUserAccounts;
    auto iter =
        std::find_if(users.begin(), users.end(), [userID](auto const &user) {
          return user->m_userID == userID;
        });
    if (iter == users.end())
      return false;
    (*iter)->m_isGlobalUser = true;
    (*iter)->setLeverage(leverage);
  }

  return true;
}

bool startGlobalBTInstance(std::function<void()> onStart,
                           std::function<void()> onEnd
#ifdef BT_USE_WITH_INDICATORS
                           ,
                           backtesting::indicator_callback_t onTick
#endif
) {
  auto bt = getGlobalBTInstance();
  auto &globalRtData = global_data_t::instance();

  if (bt == nullptr || !bt->isReady()) {
    auto const defaultConfigPath =
        std::filesystem::current_path() / "config" / "config.ini";
    if (!std::filesystem::exists(defaultConfigPath))
      return false;
    if (!createBTInstanceFromConfigFile(defaultConfigPath.string()))
      return false;
    if (bt = getGlobalBTInstance(); bt == nullptr)
      return false;

    globalRtData.onStart = std::move(onStart);
    globalRtData.onCompletion = std::move(onEnd);
#ifdef BT_USE_WITH_INDICATORS
    globalRtData.onTick = std::move(onTick);
#endif
  }
  bt->run();
  isRunning = true;
  return true;
}

bool endGlobalBTInstance() {
  auto bt = getGlobalBTInstance();
  if (bt == nullptr || !bt->isReady())
    return false;
  auto &globalRtData = global_data_t::instance();
  if (globalRtData.onCompletion)
    globalRtData.onCompletion();

  getContextObject()->stop();
#ifdef BT_USE_WITH_INDICATORS
  ticker_t::instance()->stopTimers();
#endif

  getGlobalBTInstanceImpl().reset();
  global_data_t::cleanUp();

  spdlog::info("Stopping the context object");
  std::this_thread::sleep_for(std::chrono::seconds(2));
  getContextObject()->reset();
  spdlog::info("[Done] Stopping the context object");
  isRunning = false;
  return true;
}

} // namespace backtesting

#ifdef BT_USE_WITH_INDICATORS
namespace indicators {
bool isValidIndicatorConfiguration(
    std::vector<std::vector<std::string>> const &) {
  // TODO: //check the passed data
  return true;
}
} // namespace indicators
#endif
