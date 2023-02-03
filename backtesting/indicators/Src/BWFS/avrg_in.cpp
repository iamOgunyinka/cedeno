#include "BWFS/avrg_in.hpp"

namespace indicators{

void avrg_in_callback( const backtesting::trade_data_t &trade_data, void *handler_){
    avrg_in_t *handler = (avrg_in_t*)handler_;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::buy){
        handler->common_db->cab.avrg_in = handler->common_db->cab.qty_in/handler->common_db->cab.ticks_in;
    }  
}

void avrg_in_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}