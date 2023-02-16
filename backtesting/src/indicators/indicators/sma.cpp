#include "indicators/sma.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{

void sma_callback( const kline_test_t &kline_data, 
                   indicators::indicator_t &handler_){
    indicators::sma_t &handler = *handler_.indcs_var.sma_vars;
    std::cout<<__func__<<std::endl;
    handler.n++;
    handler.sumatory += kline_data.price;
    handler.common_db->indc_info.sma.price = handler.sumatory/handler.n;
}

}