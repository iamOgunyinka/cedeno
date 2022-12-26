#include "database_connector.hpp"

#include "common.hpp"
#include <fstream>
#include <spdlog/spdlog.h>

namespace backtesting {

otl_stream &operator>>(otl_stream &os, db_token_t &item) {
  return os >> item.tokenID >> item.tradeType >> item.name >> item.baseAsset >>
         item.quoteAsset;
}

otl_stream &operator>>(otl_stream &os, db_user_order_t &item) {
  return os >> item.orderID >> item.tokenID >> item.userID >> item.quantity >>
         item.priceLevel >> item.leverage >> item.side >> item.type >>
         item.market;
}

otl_stream &operator>>(otl_stream &os, db_trade_data_t &trade) {
  return os >> trade.tradeID >> trade.orderID >> trade.quantityExec >>
         trade.amountPerPiece >> trade.tokenID >> trade.side;
}

otl_stream &operator>>(otl_stream &os, db_owned_token_t &item) {
  return os >> item.databaseID >> item.ownerID >> item.tokenID >>
         item.amountInUse >> item.amountAvailable;
}

otl_stream &operator>>(otl_stream &os, db_user_t &user) {
  return os >> user.userID;
}

void log_sql_error(otl_exception const &exception) {
  spdlog::error("SQLError code: {}", exception.code);
  spdlog::error("SQLError stmt: {}", exception.stm_text);
  spdlog::error("SQLError state: {}", (char *)exception.sqlstate);
  spdlog::error("SQLError msg: {}", (char *)exception.msg);
}

db_config_t parse_database_file(std::string const &filename,
                                std::string const &config_name) {
  std::ifstream in_file{filename};
  if (!in_file)
    return {};
  std::string line{};
  bool found = false;
  db_config_t db_config{};
  while (std::getline(in_file, line)) {
    utils::trim(line);
    if (line.empty())
      continue;

    if (line[0] == '#' && line[1] == '~') { // new config
      if (found)
        return db_config;
      found = config_name == line.c_str() + 2;
      continue;
    } else if (found) {
      auto name_pair = utils::split_string(line, ":");
      if (name_pair.size() != 2)
        continue;
      for (auto &v : name_pair)
        utils::trim(v);

      if (name_pair[0] == "username")
        db_config.username = name_pair[1];
      else if (name_pair[0] == "password")
        db_config.password = name_pair[1];
      else if (name_pair[0] == "db_dns")
        db_config.db_dns = name_pair[1];
    }
  }
  return db_config;
}

std::shared_ptr<database_connector_t>
database_connector_t::s_get_db_connector() {
  static std::shared_ptr<database_connector_t> db_connector{};
  if (!db_connector) {
    otl_connect::otl_initialize(1);
    db_connector = std::make_unique<database_connector_t>();
  }
  return db_connector;
}

void database_connector_t::username(std::string const &username) {
  db_config.username = username;
}

void database_connector_t::password(std::string const &password) {
  db_config.password = password;
}

void database_connector_t::database_name(std::string const &db_name) {
  db_config.db_dns = db_name;
}

void database_connector_t::keep_sql_server_busy() {
  spdlog::info("keeping SQL server busy");
  static std::string const s = fmt::format(
      "{}/{}@{}", db_config.username, db_config.password, db_config.db_dns);

  std::thread sql_thread{[this] {
    while (true) {
      try {
        otl_cursor::direct_exec(otl_connector_, "select 1", true);
      } catch (otl_exception const &exception) {
        log_sql_error(exception);
        otl_connector_.logoff();
        otl_connector_.rlogon(s.c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }
      std::this_thread::sleep_for(std::chrono::minutes(15));
    }
  }};
  sql_thread.detach();
}

bool database_connector_t::connect() {
  if (db_config.db_dns.empty() || db_config.password.empty() ||
      db_config.username.empty()) {
    throw std::runtime_error{"configuration incomplete"};
  }
  if (is_running)
    return is_running;

  std::string const login_str{fmt::format(
      "{}/{}@{}", db_config.username, db_config.password, db_config.db_dns)};
  try {
    this->otl_connector_.rlogon(login_str.c_str());
    keep_sql_server_busy();
    is_running = true;
  } catch (otl_exception const &exception) {
    log_sql_error(exception);
  }
  return is_running;
}

db_owned_token_list_t
database_connector_t::getOwnedTokensByUser(int const userID) {
  auto const sqlStatement =
      fmt::format("SELECT id, ownerID, tokenID, amountInUse, amountAvailable "
                  "FROM `bt_owned_tokens` WHERE ownerID={}",
                  userID);
  db_owned_token_list_t result;
  std::lock_guard<std::mutex> lockG(db_mutex_);
  try {
    otl_stream dbStream{100, sqlStatement.c_str(), otl_connector_};
    db_owned_token_t t;
    for (auto &stream : dbStream) {
      stream >> t;
      result.push_back(std::move(t));
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    result.clear();
  }
  return result;
}

db_user_order_list_t database_connector_t::getOrderForUser(int const userID) {
  auto const sqlStatement =
      fmt::format("SELECT id, tokenID, userID, quantity, priceLevel, leverage, "
                  "side, orderType, market FROM `bt_orders` WHERE userID='{}'",
                  userID);
  db_user_order_list_t result;
  std::lock_guard<std::mutex> lockG(db_mutex_);
  try {
    otl_stream dbStream{100, sqlStatement.c_str(), otl_connector_};
    db_user_order_t t;
    for (auto &stream : dbStream) {
      stream >> t;
      result.push_back(std::move(t));
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    result.clear();
  }
  return result;
}

db_trade_data_list_t database_connector_t::getTradesForUser(int const userID) {
  auto const sqlStatement =
      fmt::format("SELECT id, orderID, quantity, amount, tokenID, side "
                  "FROM `bt_trades` WHERE userID='{}'",
                  userID);
  db_trade_data_list_t result;
  std::lock_guard<std::mutex> lockG(db_mutex_);
  try {
    otl_stream dbStream{100, sqlStatement.c_str(), otl_connector_};
    db_trade_data_t t;
    for (auto &stream : dbStream) {
      stream >> t;
      result.push_back(std::move(t));
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    result.clear();
  }
  return result;
}

db_trade_data_list_t
database_connector_t::getTradesByOrderID(int const orderID) {
  auto const sqlStatement =
      fmt::format("SELECT id, orderID, quantity, amount, tokenID, side "
                  "FROM `bt_trades` WHERE orderID='{}'",
                  orderID);
  db_trade_data_list_t result;
  std::lock_guard<std::mutex> lockG(db_mutex_);
  try {
    otl_stream dbStream{100, sqlStatement.c_str(), otl_connector_};
    db_trade_data_t t;
    for (auto &stream : dbStream) {
      stream >> t;
      result.push_back(std::move(t));
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    result.clear();
  }
  return result;
}

db_token_list_t database_connector_t::getListOfAllTokens() {
  auto const sqlStatement = "SELECT id, tradeType, tokenName, baseToken, "
                            "quoteToken FROM `bt_tokens` ORDER BY tokenName";
  db_token_list_t result;
  std::lock_guard<std::mutex> lockG(db_mutex_);
  try {
    otl_stream dbStream{100, sqlStatement, otl_connector_};
    for (auto &stream : dbStream) {
      db_token_t t;
      stream >> t;
      result.push_back(std::move(t));
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    result.clear();
  }
  return result;
}

std::vector<db_user_t> database_connector_t::getUserIDs() {
  auto const sqlStatement = "SELECT id FROM `bt_users`";
  std::vector<db_user_t> userIDs;

  std::lock_guard<std::mutex> lockg{db_mutex_};
  try {
    otl_stream dbStream{10, sqlStatement, otl_connector_};
    for (auto &stream : dbStream) {
      db_user_t user;
      stream >> user.userID;
      userIDs.push_back(std::move(user));
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
  }
  return userIDs;
}

bool database_connector_t::addTokenList(db_token_list_t const &list) {
  auto const sqlStatement =
      "INSERT INTO `bt_tokens`(tradeType, tokenName) VALUES({}, '{}')";
  std::lock_guard<std::mutex> lock_g{db_mutex_};
  try {
    for (auto const &d : list) {
      auto const command = fmt::format(sqlStatement, (int)d.tradeType, d.name);
      otl_cursor::direct_exec(otl_connector_, command.c_str(),
                              otl_exception::enabled);
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    return false;
  }
  return true;
}

bool database_connector_t::addUserOwnedTokens(
    db_owned_token_list_t const &list) {
  auto const sqlStatement =
      "INSERT INTO `bt_owned_tokens`(tokenID, ownerID, amountInUse,"
      "amountAvailable) VALUES({}, {}, {}, {})";
  std::lock_guard<std::mutex> lock_g{db_mutex_};
  try {
    for (auto const &d : list) {
      auto const command = fmt::format(sqlStatement, d.tokenID, d.ownerID,
                                       d.amountInUse, d.amountAvailable);
      otl_cursor::direct_exec(otl_connector_, command.c_str(),
                              otl_exception::enabled);
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    return false;
  }
  return true;
}

bool database_connector_t::addOrderList(db_user_order_list_t const &list) {
  auto const sqlStatement =
      "INSERT INTO `bt_orders`(tokenID, userID, quantity, priceLevel,"
      "leverage, side, orderType, market) VALUES({}, {}, {}, {}, {},"
      "{}, {}, {})";
  std::lock_guard<std::mutex> lock_g{db_mutex_};
  try {
    for (auto const &d : list) {
      auto const command =
          fmt::format(sqlStatement, d.tokenID, d.userID, d.quantity,
                      d.priceLevel, d.leverage, d.side, d.type, d.market);
      otl_cursor::direct_exec(otl_connector_, command.c_str(),
                              otl_exception::enabled);
    }
  } catch (otl_exception const &e) {
    log_sql_error(e);
    return false;
  }
  return true;
}

bool database_connector_t::addNewUser(std::string const &username) {
  auto const sqlStatement =
      fmt::format("INSERT INTO `bt_users`(username) VALUES('{}')", username);
  std::lock_guard<std::mutex> lock_g{db_mutex_};
  try {
    otl_cursor::direct_exec(otl_connector_, sqlStatement.c_str(),
                            otl_exception::enabled);
  } catch (otl_exception const &e) {
    log_sql_error(e);
    return false;
  }
  return true;
}
} // namespace backtesting
