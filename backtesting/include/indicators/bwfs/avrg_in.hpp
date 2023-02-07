#ifndef AVRG_IN_HPP_
#define AVRG_IN_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"


namespace indicators{

struct avrg_in_t{
    avrg_in_t(indicators::indicator_data_t &common_db_){
        common_db = &common_db_;
    } 
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_data_t *common_db;
    indicators::ind_BWFS_confg_t configuration;
};

void avrg_in_callback( const backtesting::trade_data_t &trade_data, indicator_data_t &handler_);

}
#endif