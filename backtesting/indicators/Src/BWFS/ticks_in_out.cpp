#include "BWFS/ticks_in_out.hpp"

namespace indicators{

void ticks_in_out_callback( const backtesting::trade_data_t &trade_data, indicator_data_t &handler_){
    ticks_in_out_t &handler = *handler_.ticks_in_out_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::buy){
        handler.common_db->cab.ticks_in_out = handler.common_db->cab.ticks_in - handler.common_db->cab.ticks_out;
    }  
}

void ticks_in_out_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}