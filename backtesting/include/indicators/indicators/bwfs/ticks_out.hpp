#ifndef TICKS_OUT_HPP_
#define TICKS_OUT_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"

namespace indicators{

struct ticks_out_t{
    ticks_out_t(indicators::indicator_t &common_db_, indicators::ind_BWFS_confg_t &configuration_){
        common_db = &common_db_;
        configuration = &configuration_; 
    }  
    indicators::indicator_t *common_db;
    indicators::ind_BWFS_confg_t *configuration;
};

void ticks_out_callback(const backtesting::trade_data_t &trade_data, indicator_t &handler_);

}

#endif