#ifndef BUY_VS_SELL_HPP_
#define BUY_VS_SELL_HPP_

#include "indc_data.hpp"
#include <unordered_map>
#include <queue>
#include "indicators/bwfs/bwfs.hpp"

namespace indicators{

struct buy_vs_sell_q_t{
    buy_vs_sell_q_t(const double &quantity_,
                    const std::string &buyer_id_,
                    const std::string &seller_id_): 
        buyer_id(buyer_id_),
        seller_id(seller_id_),
        quantity(quantity_){}

    std::string buyer_id;
    std::string seller_id;
    double quantity;
};

struct buy_vs_sell_t: public bwfs_t{
    buy_vs_sell_t( indicator_t &common_db_, 
                   conf_BWFS_t &configuration_):
    bwfs_t( common_db_, configuration_){}

    ~buy_vs_sell_t(){}

    uint64_t time_threshold = 0;
    uint64_t last_time_threshold = 0;
    std::queue<buy_vs_sell_q_t> price_q;
    std::unordered_map<std::string, double> buyer;
    std::unordered_map<std::string, double> seller;
};

void buy_vs_sell_callback( const trade_stream_d &trade_data, 
                           indicator_t &handler_);
}


#endif