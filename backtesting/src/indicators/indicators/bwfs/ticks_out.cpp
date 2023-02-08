#include "indicators/bwfs/ticks_out.hpp"

namespace indicators{

void ticks_out_callback(const backtesting::trade_data_t &trade_data, indicator_t &handler_){      
    ticks_out_t &handler = *handler_.indcs_var.ticks_out_vars;
    //std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::sell){
        handler.common_db->indc_info.cab.ticks_out++;
    }
}

void ticks_out_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}