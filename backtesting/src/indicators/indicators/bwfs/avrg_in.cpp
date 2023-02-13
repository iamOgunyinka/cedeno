#include "indicators/bwfs/avrg_in.hpp"

namespace indicators{

void avrg_in_callback( const backtesting::trade_data_t &trade_data, indicator_t &handler_){
    avrg_in_t &handler = *handler_.indcs_var.avrg_in_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::buy){
        handler.common_db->indc_info.cab.avrg_in = handler.common_db->indc_info.cab.qty_in/handler.common_db->indc_info.cab.ticks_in;
    }  
}

}