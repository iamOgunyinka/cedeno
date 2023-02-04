#ifndef BWFS_HNDLR_HPP_
#define BWFS_HNDLR_HPP_
#include "indicator_data.hpp"
#include "user_data.hpp"

namespace indicators{

struct bwfs_hndlr_t{
    bwfs_hndlr_t(indicators::indicator_data_t &global_data_){
        global_data = &global_data_;
    }
    indicators::indicator_data_t *global_data;
    indicators::ind_BWFS_confg_t configuration;
    void config(const indicators::ind_BWFS_confg_t &config_);
};

void bwfs_hndlr_callback( const backtesting::trade_data_t &trade_data, 
                          void *handler_);
}
#endif