#ifndef QTY_IN_HPP_
#define QTY_IN_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"

namespace indicators{

struct qty_in_t{
    qty_in_t(indicators::indicator_t &common_db_, indicators::ind_BWFS_confg_t &configuration_){
        common_db = &common_db_;
        configuration = &configuration_; 
    }  
    indicators::indicator_t *common_db;
    indicators::ind_BWFS_confg_t *configuration;
};

void qty_in_callback(const backtesting::trade_data_t &trade_data, indicator_t &handler_);
}
#endif