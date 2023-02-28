#include "indicators/bwfs/qty_in.hpp"

namespace indicators{

void qty_in_callback(const trade_stream_d &trade_data, indicator_t &handler_){
    qty_in_t &handler = *handler_.indcs_var.qtys_in_vars;
    std::cout<<__func__<<std::endl;    
    if(trade_data.side == indicators::side_e::buy){
        handler.common_db->indc_info.cab.qty_in+= trade_data.amountPerPiece;
    }
}

}