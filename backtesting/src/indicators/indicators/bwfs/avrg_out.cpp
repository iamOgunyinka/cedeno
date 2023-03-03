#include "indicators/bwfs/avrg_out.hpp"

namespace indicators{

void avrg_out_callback( const trade_stream_d &trade_data, indicator_t &handler_){
    avrg_out_t &handler = *handler_.indcs_var.avrg_out_vars;
    std::cout<<__func__<<std::endl;
    if(trade_data.side == indicators::side_e::sell){
        handler.common_db->indc_info.cab.avrg_out = handler.common_db->indc_info.cab.qty_out/handler.common_db->indc_info.cab.ticks_out;
    }  
}
}