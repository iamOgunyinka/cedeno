#include "indicators/bwfs/ticks_in.hpp"

namespace indicators{

void ticks_in_callback(const trade_stream_d &trade_data, indicator_t &handler_){      
    ticks_in_t &handler = *handler_.indcs_var.ticks_in_vars;
    std::cout<<__func__<<std::endl;
    // if(trade_data.side == indicators::assest_side_e::buy){
        handler.common_db->info.cab.ticks_in++;
    //}
}

}