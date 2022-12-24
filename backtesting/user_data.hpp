#pragma once

#include <memory>
#include <string>
#include <vector>

namespace backtesting {

enum class trade_type_e : int {
  spot,
  futures,
};

enum class trade_side_e : int {
  sell,
  buy,
  cancel,
};

enum class trade_market_e : int {
  limit,
  market,
};

enum class order_result_e : int {
  accepted,
  rejected,
};

struct token_data_t {
  std::string name;
  double amountInUse = 0.0;
  double amountAvailable = 0.0;
  uint64_t databaseConnectID = 0;
};

struct user_order_request_t {
  token_data_t token{};
  double quantity{};
  double priceLevel{};
  double leverage = 1.0;

  trade_side_e side;
  trade_type_e type;
  trade_market_e market;
};

struct trade_data_t {
  uint64_t orderID = 0;
  uint64_t tradeID = 0;
  double quantityExec = 0.0;
  double amountPerPiece = 0.0;
  token_data_t token;
  trade_side_e side;
};

struct user_data_t {
  std::vector<token_data_t> tokens;
  std::vector<trade_data_t> trades;
  uint64_t userID = 0;

  // public:
  //  std::unique_ptr<user_order_request_t> createOrder();
};

class database_connection_t {
  //
};

using trade_list_t = std::vector<trade_data_t>;

trade_list_t initiateOrder(user_order_request_t const &order);

} // namespace backtesting
