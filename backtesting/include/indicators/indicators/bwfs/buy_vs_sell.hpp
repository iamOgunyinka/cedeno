#ifndef BUY_VS_SELL_HPP_
#define BUY_VS_SELL_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"
#include <unordered_map>
#include <queue>

namespace indicators{

struct buy_vs_sell_q_t{
    buy_vs_sell_q_t(const double &quantity_,
                    const std::string &buyer_id_,
                    const std::string &seller_id_): buyer_id(buyer_id_),
                                                    seller_id(seller_id_),
                                                    quantity(quantity_){}
    std::string buyer_id;
    std::string seller_id;
    double quantity;
};

struct buy_vs_sell_t{
    buy_vs_sell_t(indicators::indicator_t &global_data_){
        global_data = &global_data_;
    } 
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_t *global_data;
    indicators::ind_BWFS_confg_t configuration;

    uint64_t time_threshold = 0;
    uint64_t last_time_threshold = 0;
    std::queue<buy_vs_sell_q_t> price_q;
    std::unordered_map<std::string, double> buyer;
    std::unordered_map<std::string, double> seller;
};

void buy_vs_sell_callback( const backtesting::trade_data_t &trade_data, 
                           indicators::indicator_t &handler_);
}


#endif