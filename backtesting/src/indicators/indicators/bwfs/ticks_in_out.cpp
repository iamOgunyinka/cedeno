#include "indicators/bwfs/ticks_in_out.hpp"

namespace indicators{

void tick_in_out_callback( const trade_stream_d &trade_data, indicator_t &handler_){
    ticks_in_out_t &handler = *handler_.indcs_var.ticks_in_out_vars;
    std::cout<<__func__<<std::endl;
    handler.common_db->info.cab.ticks_in_out = handler.common_db->info.cab.ticks_in - handler.common_db->info.cab.ticks_out;
}

}