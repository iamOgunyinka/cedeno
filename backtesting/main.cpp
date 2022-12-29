// backtesting.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include <CLI11/CLI11.hpp>
#include <boost/asio/io_context.hpp>
#include <random>
#include <spdlog/spdlog.h>

#include "adaptor.hpp"
#include "argument.hpp"
#include "candlestick_data.hpp"
#include "database_connector.hpp"
#include "depth_data.hpp"

backtesting::global_data_t globalUserData;

void processBookTickerStream(backtesting::trade_map_td const &tradeMap) {
  //
}

void processTickerStream(backtesting::trade_map_td const &tradeMap) {
  //
}

void setupDummyList(backtesting::db_token_list_t const &tokenList,
                    backtesting::database_connector_t &dbConnector,
                    std::vector<backtesting::db_user_t> &userList) {
  using backtesting::trade_market_e;
  using backtesting::trade_side_e;
  using backtesting::trade_type_e;

  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<> uid(0, tokenList.size());

  assert(dbConnector.addNewUser("joshua"));
  userList = dbConnector.getUserIDs();
  assert(!userList.empty());
  assert(userList[0].userID == 1);

  backtesting::db_user_t &user = userList[0];

  backtesting::db_owned_token_list_t ownedTokens;
  ownedTokens.reserve(20);
  for (int i = 0; i < 20; ++i) {
    auto &token = tokenList[uid(gen)];
    backtesting::db_owned_token_t ownToken;
    ownToken.ownerID = user.userID;
    ownToken.tokenID = token.tokenID;
    ownToken.amountAvailable = 500.0;
    ownedTokens.push_back(std::move(ownToken));
  }

  backtesting::db_user_order_list_t orderList;
  orderList.reserve(10);

  for (int i = 0; i < 10; ++i) {
    auto const isEven = i % 5 == 0;
    auto &token = tokenList[uid(gen)];
    backtesting::db_user_order_t order;
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

void saveTokenFromFileToDatabase(backtesting::database_connector_t &dbConnector,
                                 backtesting::db_token_list_t &dbTokenList,
                                 std::string const &rootPath) {
  using backtesting::trade_type_e;
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
      backtesting::utils::trim(line);
      if (!line.empty()) {
        backtesting::db_token_list_t::value_type d;
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

using backtesting::utils::currentTimeToString;
using backtesting::utils::listContains;
using backtesting::utils::stringToTimeT;

int main(int argc, char **argv) {
  CLI::App app{"backtesting software for Creed & Bear LLC"};
  argument_t args;
  using backtesting::stringlist_t;

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

  CLI11_PARSE(app, argc, argv);
#ifdef _DEBUG
  spdlog::info("start date: {}", args.dateFromStr);
  spdlog::info("end date: {}", args.dateToStr);
  spdlog::info("rootDir: {}", args.rootDir);

  for (auto const &token : args.tokenList)
    spdlog::info("token: {}", token);
  for (auto const &s : args.streams)
    spdlog::info("stream: {}", s);
  for (auto const &t : args.tradeTypes)
    spdlog::info("trade: {}", t);

#endif // _DEBUG

  if (args.dbLaunchType.empty())
    args.dbLaunchType = "development";

  if (args.dbConfigFilename.empty())
    args.dbConfigFilename = backtesting::getDatabaseConfigPath();

  auto const dbConfig = backtesting::parse_database_file(args.dbConfigFilename,
                                                         args.dbLaunchType);
  if (!dbConfig) {
    spdlog::error("Unable to get database configuration values");
    return EXIT_FAILURE;
  }

  if (args.streams.empty()) {
    args.streams.push_back(DEPTH);
  } else {
    std::vector<std::string> const validStreams{TRADE, TICKER, BTICKER,
                                                CANDLESTICK, DEPTH};
    for (auto const &stream : args.streams) {
      if (!listContains(validStreams, stream)) {
        spdlog::error("'{}' is not a valid stream type", stream);
        return EXIT_FAILURE;
      }
    }
  }

  if (args.tradeTypes.empty()) {
    args.tradeTypes.push_back(SPOT);
  } else {
    std::vector<std::string> const validTrades{SPOT, FUTURES};
    for (auto const &trade : args.tradeTypes) {
      if (!listContains(validTrades, trade)) {
        spdlog::error("'{}' is not a valid trade type", trade);
        return EXIT_FAILURE;
      }
    }
  }

  if (args.tokenList.empty()) {
    args.tokenList.push_back("BTCUSDT");
#ifdef _DEBUG
    args.tokenList.push_back("ETHUSDT");
#endif // _DEBUG
  }

  if (args.rootDir.empty())
#ifdef _DEBUG
    args.rootDir = "D:\\Visual Studio "
                   "Projects\\cedeno\\test_data_extractor\\backtestingFiles";
#else
    rootDir = ".";
#endif // _DEBUG

  if (!std::filesystem::exists(args.rootDir)) {
    spdlog::error("'{}' does not exist.", args.rootDir);
    return EXIT_FAILURE;
  }

#ifdef _DEBUG
  if (args.dateFromStr.empty()) {
    constexpr std::size_t const last24hrs = 3'600 * 24;
    args.dateFromStr = fmt::format(
        "{} 00:00:00",
        currentTimeToString(std::time(nullptr) - last24hrs, "-").value());
  }

  if (args.dateToStr.empty()) {
    args.dateToStr = fmt::format(
        "{} 11:59:59", currentTimeToString(std::time(nullptr), "-").value());
  }
#endif // _DEBUG

  std::time_t startTime = 0, endTime = 0;
  if (auto const optStartTime = stringToTimeT(args.dateFromStr);
      optStartTime.has_value()) {
    startTime = *optStartTime;
  } else {
    spdlog::error("Unable to calculate the start date from user input");
    return EXIT_FAILURE;
  }

  if (auto const optEndTime = stringToTimeT(args.dateToStr);
      optEndTime.has_value()) {
    endTime = *optEndTime;
  } else {
    spdlog::error("Unable to calculate the end date from user input");
    return EXIT_FAILURE;
  }

  if (startTime > endTime)
    std::swap(startTime, endTime);

  auto const csvFilenames = backtesting::utils::getListOfCSVFiles(
      args.tokenList, args.tradeTypes, args.streams, startTime, endTime,
      args.rootDir);
  if (csvFilenames.empty()) {
    spdlog::error("No files found matching that criteria");
    return EXIT_FAILURE;
  }

  auto databaseConnector =
      backtesting::database_connector_t::s_get_db_connector();
  databaseConnector->username(dbConfig.username);
  databaseConnector->password(dbConfig.password);
  databaseConnector->database_name(dbConfig.db_dns);
  if (!databaseConnector->connect())
    return EXIT_FAILURE;

  auto dbTokenList = databaseConnector->getListOfAllTokens();
  if (dbTokenList.empty())
    saveTokenFromFileToDatabase(*databaseConnector, dbTokenList, args.rootDir);
  auto dbUserList = databaseConnector->getUserIDs();
  if (dbUserList.empty())
    setupDummyList(dbTokenList, *databaseConnector, dbUserList);

  globalUserData.tokens =
      backtesting::adaptor::dbTokenListToBtTokenList(dbTokenList);
  auto &users = globalUserData.userAccounts;
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

  if (auto const iter = csvFilenames.find(BTICKER);
      iter != csvFilenames.cend()) {
    std::thread{[bookTickerInfo = iter->second] {
      processBookTickerStream(bookTickerInfo);
    }}.detach();
  }

  if (auto const iter = csvFilenames.find(TICKER);
      iter != csvFilenames.cend()) {
    std::thread{[tickerInfo = iter->second] {
      processTickerStream(tickerInfo);
    }}.detach();
  }

  if (auto const iter = csvFilenames.find(CANDLESTICK);
      iter != csvFilenames.cend()) {
    std::thread{[csData = iter->second] {
      backtesting::processCandlestickStream(csData);
    }}.detach();
  }

  net::io_context ioContext;
  if (auto iter = csvFilenames.find(DEPTH); iter != csvFilenames.end()) {
    std::thread{[csData = iter->second, &ioContext]() mutable {
      backtesting::processDepthStream(ioContext, csData);
    }}.detach();
  }

  std::this_thread::sleep_for(std::chrono::seconds(100));
  ioContext.run();
  return 0;
}
