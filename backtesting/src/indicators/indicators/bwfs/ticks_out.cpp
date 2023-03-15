#include "indicators/bwfs/ticks_out.hpp"

namespace indicators{

void ticks_out_callback(const trade_stream_d &trade_data, indicator_t &handler_){      
    ticks_out_t &handler = *handler_.indcs_var.ticks_out_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == indicators::assest_side_e::sell){
        handler.common_db->info.cab.ticks_out++;
    }
}

}