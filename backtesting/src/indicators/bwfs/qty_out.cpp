#include "bwfs/qty_out.hpp"

namespace indicators{

void qty_out_callback(const backtesting::trade_data_t &trade_data, indicator_t &handler_){
    qty_out_t &handler = *handler_.indcs_var.qty_out_vars;
    //std::cout<<__func__<<std::endl;    
    if(trade_data.side == backtesting::trade_side_e::sell){
        handler.common_db->indc_info.cab.qty_out+= trade_data.amountPerPiece;
    }
}

void qty_out_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}