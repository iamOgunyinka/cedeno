#ifndef BWFS_HNDLR_HPP_
#define BWFS_HNDLR_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"

namespace indicators{

struct bwfs_hndlr_t{
    bwfs_hndlr_t(indicators::indicator_data_t &global_data_){
        global_data = &global_data_;
    }
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_data_t *global_data;
    indicators::ind_BWFS_confg_t configuration;
    bool set_threshold;
    uint64_t time_threshold;

};

void bwfs_hndlr_callback( const backtesting::trade_data_t &trade_data, 
                          indicator_data_t &handler_);
}
#endif