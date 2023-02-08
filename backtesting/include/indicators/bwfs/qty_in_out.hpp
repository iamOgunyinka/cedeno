#ifndef QTY_IN_OUT_HPP_
#define QTY_IN_OUT_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"

namespace indicators{

struct qty_in_out_t{
    qty_in_out_t(indicators::indicator_t &common_db_){
        common_db = &common_db_;
    } 
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_t *common_db;
    indicators::ind_BWFS_confg_t configuration;
};



void qty_in_out_callback( const backtesting::trade_data_t &trade_data, indicator_t &handler_);

}

#endif