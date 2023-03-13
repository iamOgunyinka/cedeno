#include "arguments_parser.hpp"

#ifdef BT_USE_WITH_DB
#include "adaptor.hpp"
#include "database_connector.hpp"
#endif // BT_USE_WITH_DB

#include "global_data.hpp"
#include <CLI11/CLI11.hpp>
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

  if (config.tradeTypes.empty()) {
    PRINT_INFO("trade type not specified, will use 'SPOT' as default")
    config.tradeTypes.push_back(SPOT);
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    for (auto &trade : config.tradeTypes) {
      if (!listContains(validTrades, trade)) {
        ERROR_EXIT("'{}' is not a valid trade type", trade);
      }
      for (auto &t : trade)
        t = tolower(t);
    }
  }

  if (config.tokenList.empty()) {
    PRINT_INFO("token list is empty, using 'BTCUSDT' as the default");
    config.tokenList.push_back("BTCUSDT");
#ifdef _DEBUG
    PRINT_INFO("adding 'ETHUSDT' to the token list");
    config.tokenList.push_back("ETHUSDT");
#endif // _DEBUG
  } else {
    for (auto &token : config.tokenList) {
      for (auto &t : token)
        t = toupper(t);
    }
  }

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
      config.tokenList, config.tradeTypes, config.streams,
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

  if (verbose) {
    spdlog::info("start date: {}", config.dateFromStr);
    spdlog::info("end date: {}", config.dateToStr);
    spdlog::info("rootDir: {}", config.rootDir);

    for (auto const &token : config.tokenList)
      spdlog::info("token: {}", token);
    for (auto const &s : config.streams)
      spdlog::info("stream: {}", s);
    for (auto const &t : config.tradeTypes)
      spdlog::info("trade: {}", t);
  }

  m_config.emplace(config);
  m_argumentParsed = true;
  return m_argumentParsed;
}

bool backtesting_t::parse(int argc, char **argv) {
  m_argumentParsed = false;

  CLI::App app{"backtesting software for Creed & Bear LLC"};
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
  app.add_option("--trade-types", args.tradeTypes,
                 "A list of trade types. Valid options are: "
                 "[futures, spot(default)]");
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

std::optional<backtesting_t>
newBTInstance(backtesting::configuration_t const &config) {
  backtesting_t bt;
  if (!(bt.parseImpl(config) && bt.prepareData() && bt.isReady()))
    return std::nullopt;
  return bt;
}
