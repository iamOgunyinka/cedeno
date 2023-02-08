#ifndef TICKS_IN_HPP_
#define TICKS_IN_HPP_

#include "user_data.hpp"
#include "indc_data.hpp"

namespace indicators{

struct ticks_in_t{
    ticks_in_t(indicators::indicator_t &common_db_){
        common_db = &common_db_;
    } 
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_t *common_db;
    indicators::ind_BWFS_confg_t configuration;
};

void ticks_in_callback(const backtesting::trade_data_t &trade_data, indicator_t &handler_);

}
#endif