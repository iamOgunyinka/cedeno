#include "indicators/bwfs/avrg_in.hpp"

namespace indicators{

void avrg_in_callback( const trade_stream_d &trade_data, indicator_t &handler_){
    avrg_in_t &handler = *handler_.indcs_var.avrg_in_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == indicators::assest_side_e::buy){
        handler.common_db->info.cab.avrg_in = handler.common_db->info.cab.qty_in/handler.common_db->info.cab.ticks_in;
    }  
}

}