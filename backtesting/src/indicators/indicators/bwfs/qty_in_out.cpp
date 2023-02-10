#include "indicators/bwfs/qty_in_out.hpp"

namespace indicators{


void qty_in_out_callback( const backtesting::trade_data_t &trade_data, indicator_t &handler_){
    qty_in_out_t &handler = *handler_.indcs_var.qty_in_out_vars;
    std::cout<<__func__<<std::endl;
    handler.common_db->indc_info.cab.qty_in_out = handler.common_db->indc_info.cab.qty_in - handler.common_db->indc_info.cab.qty_out;
}

void qty_in_out_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}