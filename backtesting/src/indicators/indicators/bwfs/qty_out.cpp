#include "indicators/bwfs/qty_out.hpp"

namespace indicators{

void qty_out_callback(const trade_stream_d &trade_data, indicator_t &handler_){
    qty_out_t &handler = *handler_.indcs_var.qty_out_vars;
    std::cout<<__func__<<std::endl;    
    if(trade_data.side == indicators::assest_side_e::sell){
        handler.common_db->info.cab.qty_out+= trade_data.amountPerPiece;
    }
}

}