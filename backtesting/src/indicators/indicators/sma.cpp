#include "indicators/sma.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
void sma_callback( const kline_test_t &kline_data, 
                   indicators::indicator_t &handler_){
    indicators::sma_t &handler = *handler_.indcs_var.sma_vars;
    std::cout<<__func__<<std::endl;
    if(handler.n < handler.configuration->n){
        handler.n++;
    }else{
        const double &back_price = handler.prices_q.back();
        handler.sumatory -= back_price;
        handler.prices_q.pop();
    }
    handler.sumatory += kline_data.price;
    handler.common_db->indc_info.sma.price = handler.sumatory/handler.n;
    handler.prices_q.push(kline_data.price);
}
}