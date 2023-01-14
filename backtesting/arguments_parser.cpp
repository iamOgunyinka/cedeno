#include "arguments_parser.hpp"

#include "adaptor.hpp"

#ifdef BT_USE_WITH_DB
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
using backtesting::utils::listContains;
using backtesting::utils::stringToTimeT;

bool verbose =
#ifdef _DEBUG
    true;
#else
    false;
#endif // _DEBUG

namespace backtesting {
#ifdef BT_USE_WITH_DB
void setupDummyList(db_token_list_t const &tokenList,
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
  assets.reserve(20);

  for (int i = 0; i < 20; ++i) {
    auto &token = tokenList[uid(gen)];
    db_user_asset_list_t::value_type userAsset;
    userAsset.ownerID = user.userID;
    userAsset.tokenID = token.tokenID;
    userAsset.amountAvailable = 500.0;
    assets.push_back(std::move(userAsset));
  }

  db_user_order_list_t orderList;
  orderList.reserve(10);

  for (int i = 0; i < 10; ++i) {
    auto const isEven = i % 5 == 0;
    auto &token = tokenList[uid(gen)];
    db_user_order_t order;
    order.leverage = 10.0;
    order.market = int(isEven ? trade_market_e::limit : trade_market_e::market);
    order.priceLevel = double(i);
    order.quantity = double(i) + 1.0;
    order.side = int(isEven ? trade_side_e::buy : trade_side_e::sell);
    order.tokenID = token.tokenID;
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
        db_token_list_t::value_type d;
        d.name = line;
        d.tradeType = (int)tradeType;
        dbTokenList.push_back(std::move(d));
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
      token_data_t d;
      d.name = line;
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

  if (!std::filesystem::exists(dirPath))
    return result;

  for (auto const &[tradeType, filename] : pair) {
    auto const path = dirPath / filename;
    if (!std::filesystem::exists(filename))
      continue;
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

  if (config.streams.empty()) {
    PRINT_INFO("Streams not specified, will use 'DEPTH' as default")
    config.streams.push_back(DEPTH);
  } else {
    std::vector<std::string> const validStreams{TRADE, TICKER, BTICKER,
                                                CANDLESTICK, DEPTH};
    for (auto const &stream : config.streams) {
      if (!listContains(validStreams, stream)) {
        ERROR_EXIT("'{}' is not a valid stream type", stream)
      }
    }
  }

  if (config.tradeTypes.empty()) {
    PRINT_INFO("trade type not specified, will use 'SPOT' as default")
    config.tradeTypes.push_back(SPOT);
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    for (auto const &trade : config.tradeTypes) {
      if (!listContains(validTrades, trade)) {
        ERROR_EXIT("'{}' is not a valid trade type", trade);
      }
    }
  }

  if (config.tokenList.empty()) {
    PRINT_INFO("token list is empty, using 'BTCUSDT' as the default");
    config.tokenList.push_back("BTCUSDT");
#ifdef _DEBUG
    PRINT_INFO("adding 'ETHUSDT' to the token list");
    config.tokenList.push_back("ETHUSDT");
#endif // _DEBUG
  }

  if (config.rootDir.empty()) {
    config.rootDir =
        (std::filesystem::current_path() / "backtestingFiles").string();
  }

  if (!std::filesystem::exists(config.rootDir)) {
    ERROR_EXIT("'{}' does not exist.", config.rootDir);
  }

#ifdef _DEBUG
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
#endif // _DEBUG

  auto &globalRtData = global_data_t::instance();

  if (auto const optStartTime = stringToTimeT(config.dateFromStr);
      optStartTime.has_value()) {
    globalRtData.startTime = *optStartTime;
  } else {
    ERROR_EXIT("Unable to calculate the start date from user input");
  }

  if (auto const optEndTime = stringToTimeT(config.dateToStr);
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

  m_argumentParsed = true;
  return m_argumentParsed;
}

bool backtesting_t::parse(size_t argc, char **argv) {
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
  if (dbTokenList.empty())
    saveTokenFromFileToDatabase(*databaseConnector, dbTokenList,
                                m_config->rootDir);
  auto dbUserList = databaseConnector->getUserIDs();
  if (dbUserList.empty())
    setupDummyList(dbTokenList, *databaseConnector, dbUserList);

  globalRtData.allTokens =
      backtesting::adaptor::dbTokenListToBtTokenList(dbTokenList);
  auto &users = globalRtData.allUserAccounts;
  users.clear();
  users.reserve(dbUserList.size());

  for (auto const &u : dbUserList) {
    auto user = std::make_shared<backtesting::user_data_t>();
    user->assets = backtesting::adaptor::dbUserAssetsToBtUserAssets(
        databaseConnector->getAllAssetsByUser(u.userID), dbTokenList);
    user->orders = backtesting::adaptor::dbOrderListToBtOrderList(
        databaseConnector->getOrderForUser(u.userID), dbTokenList, user.get());
    user->trades = backtesting::adaptor::dbTradeListToBtTradeList(
        databaseConnector->getTradesForUser(u.userID), dbTokenList);
    users.push_back(std::move(user));
  }

#else
  globalRtData.allTokens = backtesting::readTokensFromFile(m_config->rootDir);

#endif

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
