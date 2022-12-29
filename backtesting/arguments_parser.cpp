#include "arguments_parser.hpp"

#include "adaptor.hpp"
#include "common.hpp"
#include "database_connector.hpp"
#include <CLI11/CLI11.hpp>
#include <pybind11/pybind11.h>
#include <random>
#include <spdlog/spdlog.h>

#define ERROR_PARSE() (m_args.reset(), m_argumentParsed)
#define PRINT_INFO(str, ...)                                                   \
  {                                                                            \
    if (verbose)                                                               \
      spdlog::info(str, ##__VA_ARGS__);                                        \
  }

using backtesting::utils::currentTimeToString;
using backtesting::utils::listContains;
using backtesting::utils::stringToTimeT;

bool verbose =
#ifdef _DEBUG
    true;
#else
    false;
#endif // _DEBUG

extern global_data_t globalRtData;

namespace backtesting {
void setupDummyList(db_token_list_t const &tokenList,
                    database_connector_t &dbConnector,
                    std::vector<db_user_t> &userList) {
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<> uid(0, tokenList.size());

  assert(dbConnector.addNewUser("joshua"));
  userList = dbConnector.getUserIDs();
  assert(!userList.empty());
  assert(userList[0].userID == 1);

  db_user_t &user = userList[0];
  db_owned_token_list_t ownedTokens;
  ownedTokens.reserve(20);

  for (int i = 0; i < 20; ++i) {
    auto &token = tokenList[uid(gen)];
    db_owned_token_t ownToken;
    ownToken.ownerID = user.userID;
    ownToken.tokenID = token.tokenID;
    ownToken.amountAvailable = 500.0;
    ownedTokens.push_back(std::move(ownToken));
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

  assert(dbConnector.addUserOwnedTokens(ownedTokens));
  assert(dbConnector.addOrderList(orderList));
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
} // namespace backtesting

bool argument_parser_t::parse(std::vector<std::string> argv) {
  m_argumentParsed = false;

  CLI::App app{"backtesting software for Creed & Bear LLC"};
  auto &args = m_args.emplace();

  app.add_option("--db-config", args.dbConfigFilename,
                 fmt::format("database configuration filename "
                             "(default: '{}')",
                             args.dbConfigFilename));
  app.add_option("--db-launch-type", args.dbLaunchType,
                 fmt::format("database configuration"
                             " launch type(default: '{}')",
                             args.dbLaunchType));
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
    app.parse(argv);
  } catch (const CLI::ParseError &e) {
    spdlog::error(e.what());
    return false;
  }

  if (verbose) {
    spdlog::info("start date: {}", args.dateFromStr);
    spdlog::info("end date: {}", args.dateToStr);
    spdlog::info("rootDir: {}", args.rootDir);

    for (auto const &token : args.tokenList)
      spdlog::info("token: {}", token);
    for (auto const &s : args.streams)
      spdlog::info("stream: {}", s);
    for (auto const &t : args.tradeTypes)
      spdlog::info("trade: {}", t);
  }

  if (args.dbLaunchType.empty())
    args.dbLaunchType = "development";

  if (args.dbConfigFilename.empty())
    args.dbConfigFilename = backtesting::getDatabaseConfigPath();

  auto const dbConfig = backtesting::parse_database_file(args.dbConfigFilename,
                                                         args.dbLaunchType);
  if (!dbConfig) {
    spdlog::error("Unable to get database configuration values");
    return ERROR_PARSE();
  }

  if (args.streams.empty()) {
    PRINT_INFO("Streams not specified, will use 'DEPTH' as default")
    args.streams.push_back(DEPTH);
  } else {
    std::vector<std::string> const validStreams{TRADE, TICKER, BTICKER,
                                                CANDLESTICK, DEPTH};
    for (auto const &stream : args.streams) {
      if (!listContains(validStreams, stream)) {
        spdlog::error("'{}' is not a valid stream type", stream);
        return ERROR_PARSE();
      }
    }
  }

  if (args.tradeTypes.empty()) {
    PRINT_INFO("trade type not specified, will use 'SPOT' as default")
    args.tradeTypes.push_back(SPOT);
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    for (auto const &trade : args.tradeTypes) {
      if (!listContains(validTrades, trade)) {
        spdlog::error("'{}' is not a valid trade type", trade);
        return ERROR_PARSE();
      }
    }
  }

  if (args.tokenList.empty()) {
    PRINT_INFO("token list is empty, using 'BTCUSDT' as the default");
    args.tokenList.push_back("BTCUSDT");
#ifdef _DEBUG
    PRINT_INFO("adding 'ETHUSDT' to the token list");
    args.tokenList.push_back("ETHUSDT");
#endif // _DEBUG
  }

  if (args.rootDir.empty())
#ifdef _DEBUG
    args.rootDir = "D:\\Visual Studio "
                   "Projects\\cedeno\\test_data_extractor\\backtestingFiles";
#else
    args.rootDir = ".";
#endif // _DEBUG

  if (!std::filesystem::exists(args.rootDir)) {
    spdlog::error("'{}' does not exist.", args.rootDir);
    return ERROR_PARSE();
  }

#ifdef _DEBUG
  if (args.dateFromStr.empty()) {
    constexpr std::size_t const last24hrs = 3'600 * 24;
    args.dateFromStr = fmt::format(
        "{} 00:00:00",
        currentTimeToString(std::time(nullptr) - last24hrs, "-").value());
    PRINT_INFO("Start date not specified, will use '{}'", args.dateFromStr)
  }

  if (args.dateToStr.empty()) {
    args.dateToStr = fmt::format(
        "{} 23:59:59", currentTimeToString(std::time(nullptr), "-").value());
    PRINT_INFO("End-date not specified, will use '{}'", args.dateToStr)
  }
#endif // _DEBUG

  if (auto const optStartTime = stringToTimeT(args.dateFromStr);
      optStartTime.has_value()) {
    globalRtData.startTime = *optStartTime;
  } else {
    spdlog::error("Unable to calculate the start date from user input");
    return ERROR_PARSE();
  }

  if (auto const optEndTime = stringToTimeT(args.dateToStr);
      optEndTime.has_value()) {
    globalRtData.endTime = *optEndTime;
  } else {
    spdlog::error("Unable to calculate the end date from user input");
    return ERROR_PARSE();
  }

  if (globalRtData.startTime > globalRtData.endTime)
    std::swap(globalRtData.startTime, globalRtData.endTime);

  globalRtData.listOfFiles = backtesting::utils::getListOfCSVFiles(
      args.tokenList, args.tradeTypes, args.streams, globalRtData.startTime,
      globalRtData.endTime, args.rootDir);

  if (globalRtData.listOfFiles.empty()) {
    spdlog::error("No files found matching the user-defined criteria");
    return ERROR_PARSE();
  }

  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  databaseConnector->username(dbConfig.username);
  databaseConnector->password(dbConfig.password);
  databaseConnector->database_name(dbConfig.db_dns);
  if (!databaseConnector->connect())
    return ERROR_PARSE();

  m_argumentParsed = true;
  return m_argumentParsed;
}

bool argument_parser_t::prepareData() {
  if (!m_argumentParsed) {
    spdlog::error(
        "The user-settings has not been parsed yet, call .parse on the object");
    return m_argumentParsed;
  }

  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  auto dbTokenList = databaseConnector->getListOfAllTokens();
  if (dbTokenList.empty())
    saveTokenFromFileToDatabase(*databaseConnector, dbTokenList,
                                m_args->rootDir);
  auto dbUserList = databaseConnector->getUserIDs();
  if (dbUserList.empty())
    setupDummyList(dbTokenList, *databaseConnector, dbUserList);

  globalRtData.allTokens =
      backtesting::adaptor::dbTokenListToBtTokenList(dbTokenList);
  auto &users = globalRtData.allUserAccounts;
  users.clear();
  users.reserve(dbUserList.size());

  for (auto const &u : dbUserList) {
    backtesting::user_data_t user;
    user.tokensOwned = backtesting::adaptor::dbOwnedTokenListToBtOwnedToken(
        databaseConnector->getOwnedTokensByUser(u.userID), dbTokenList);
    user.orders = backtesting::adaptor::dbOrderListToBtOrderList(
        databaseConnector->getOrderForUser(u.userID), dbTokenList);
    user.trades = backtesting::adaptor::dbTradeListToBtTradeList(
        databaseConnector->getTradesForUser(u.userID), dbTokenList);
    users.push_back(std::move(user));
  }

  m_authenticatedData = true;
  return true;
}

namespace py = pybind11;

PYBIND11_MODULE(backtester, m) {
  py::class_<argument_parser_t>(m, "argument_parser_t")
      .def("parse", &argument_parser_t::parse)
      .def("prepareData", &argument_parser_t::prepareData)
      .def("isReady", &argument_parser_t::isReady)
      .def("runBacktester", &argument_parser_t::runBacktester);
}
