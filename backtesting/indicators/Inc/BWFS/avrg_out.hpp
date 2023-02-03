#ifndef AVRG_OUT_HPP_
#define AVRG_OUT_HPP_
#include "user_data.hpp"
#include "indicator_data.hpp"

namespace indicators{

struct avrg_out_t{
    avrg_out_t(indicators::indicator_data_t &common_db_){
        common_db = &common_db_;
    } 
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_data_t *common_db;
    indicators::ind_BWFS_confg_t configuration;
};

void avrg_out_callback( const backtesting::trade_data_t &trade_data, void *handler);

}

#endif