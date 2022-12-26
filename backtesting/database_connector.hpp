#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "user_data.hpp"

#define OTL_BIG_INT long long
#define OTL_ODBC
#define OTL_ODBC_MYSQL
#define OTL_STL
#ifdef _WIN32
#define OTL_ODBC_WINDOWS
#else
#define OTL_ODBC_UNIX
#endif
#define OTL_SAFE_EXCEPTION_ON
#include <otl_v4/otlv4.h>

namespace backtesting {

struct db_token_t {
  int tokenID = 0;
  int tradeType = 0;
  std::string name;
  std::string baseAsset;
  std::string quoteAsset;
};
using db_token_list_t = std::vector<db_token_t>;

struct db_owned_token_t {
  int databaseID = 0;
  int ownerID = 0;
  int tokenID = 0;
  double amountInUse = 0.0;
  double amountAvailable = 0.0;
};
using db_owned_token_list_t = std::vector<db_owned_token_t>;

struct db_user_t {
  int userID = 0;
};

struct db_user_order_t {
  int orderID = 0;
  int tokenID = 0;
  int userID = 0;
  double quantity = 0.0;
  double priceLevel = 0.0;
  double leverage = 1.0;

  int side = 0;
  int type = 0;
  int market = 0;
};
using db_user_order_list_t = std::vector<db_user_order_t>;

struct db_trade_data_t {
  int tradeID = 0;
  int orderID = 0;
  int tokenID = 0;
  int side = 0;
  double quantityExec = 0.0;
  double amountPerPiece = 0.0;
};
using db_trade_data_list_t = std::vector<db_trade_data_t>;

struct db_config_t {
  std::string db_dns;   // could be SQLite3
  std::string username; // not-empty but ignored if SQLite3
  std::string password; // not-empty ignored if SQLite3

  operator bool() const {
    return !(db_dns.empty() && username.empty() && password.empty());
  }
};

struct database_connector_t {
  db_config_t db_config;
  otl_connect otl_connector_;
  std::mutex db_mutex_;
  bool is_running = false;

private:
  void keep_sql_server_busy();

public:
  static std::shared_ptr<database_connector_t> s_get_db_connector();
  void username(std::string const &username);
  void password(std::string const &password);
  void database_name(std::string const &db_name);
  [[nodiscard]] bool connect();

  [[nodiscard]] std::vector<db_user_t> getUserIDs();
  [[nodiscard]] db_token_list_t getListOfAllTokens();
  [[nodiscard]] db_user_order_list_t getOrderForUser(int const userID);
  [[nodiscard]] db_trade_data_list_t getTradesForUser(int const userID);
  [[nodiscard]] db_owned_token_list_t getOwnedTokensByUser(int const userID);
  [[nodiscard]] std::vector<db_trade_data_t>
  getTradesByOrderID(int const orderID);

  bool addTokenList(db_token_list_t const &);
  bool addOrderList(db_user_order_list_t const &);
  bool addUserOwnedTokens(db_owned_token_list_t const &);
  bool addNewUser(std::string const &username);
};

otl_stream &operator>>(otl_stream &, db_token_t &);
otl_stream &operator>>(otl_stream &, db_user_order_t &);
otl_stream &operator>>(otl_stream &, db_trade_data_t &);
otl_stream &operator>>(otl_stream &, db_owned_token_t &);
otl_stream &operator>>(otl_stream &, db_user_t &);
void log_sql_error(otl_exception const &exception);
[[nodiscard]] db_config_t parse_database_file(std::string const &filename,
                                              std::string const &config_name);

} // namespace backtesting
