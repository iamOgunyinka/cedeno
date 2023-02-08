#ifndef BWFS_HNDLR_HPP_
#define BWFS_HNDLR_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"
#include <queue>

namespace indicators{

struct bwfs_hndlr_t{
    bwfs_hndlr_t(indicators::indicator_t &global_data_){
        global_data = &global_data_;
    }
    void config(const indicators::ind_BWFS_confg_t &config_);
    indicators::indicator_t *global_data;
    indicators::ind_BWFS_confg_t configuration;
    uint64_t time_threshold = 0;
    uint64_t last_time_threshold = 0;
    indicator_info_lis_t indc_data_q;

};

void bwfs_hndlr_callback( const backtesting::trade_data_t &trade_data, 
                          indicator_t &handler_);
}
#endif