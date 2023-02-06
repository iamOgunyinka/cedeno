#include "BWFS/ticks_in.hpp"

namespace indicators{

void ticks_in_callback(const backtesting::trade_data_t &trade_data, indicator_data_t &handler_){      
    ticks_in_t handler = *handler_.ticks_in_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::buy){
        handler.common_db->cab.ticks_in++;
    }
}

void ticks_in_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}