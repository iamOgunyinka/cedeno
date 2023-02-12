#include "bwfs/qty_in_out.hpp"

namespace indicators{


void qty_in_out_callback( const backtesting::trade_data_t &trade_data, indicator_data_t &handler_){
    qty_in_out_t &handler = *handler_.qty_in_out_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::buy){
        handler.common_db->cab.qty_in_out = handler.common_db->cab.qty_in - handler.common_db->cab.qty_out;
    }  
}

void qty_in_out_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}