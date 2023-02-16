#include "indicators/ema.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{

static void ema_calculate( indicators::ema_t &handler, 
                           const double &curr_price){
    double k = 2/(handler.configuration->n + 1);
    double &ema_price = handler.common_db->indc_info.ema.price;
    ema_price = k*(curr_price - ema_price) + ema_price;
}

static bool sma_calculate( indicators::ema_t &handler,
                           const backtesting::trade_data_t &trade_data){
    handler.SMA.sumatory += trade_data.amountPerPiece;
    if(++handler.SMA.counter == 30){
        handler.common_db->indc_info.ema.price = handler.SMA.sumatory/handler.SMA.counter;
        return true;
    }
    return false;
}

void ema_callback( const backtesting::trade_data_t &trade_data, 
                   indicators::indicator_t &handler_){
    indicators::ema_t handler = *handler_.indcs_var.ema_vars;
    std::cout<<__func__<<std::endl;
    if(handler.SMA.calculating == true){
        if(sma_calculate(handler, trade_data))
            handler.SMA.calculating = false;
    }else{
        ema_calculate(handler, trade_data.amountPerPiece);
    }    
}

namespace config{
namespace ema{
conf_ema_t get_config(const std::vector<std::string> &indcs){
    conf_ema_t config;
    if(indcs.size() > 1){
        auto config_pair = indicators::indcs_utils::split_string(indcs[1], ":");
        if(config_pair.first == "n"){
            if(!indicators::indcs_utils::check_if_string_is_number(config_pair.second)){
                std::__throw_runtime_error("Wrong ema config, n must be a number");
            }
            config.n = strtoul(config_pair.second.c_str(), nullptr, 10);
            if(config.n < 1){
                std::__throw_runtime_error("Wrong ema config, n must be greater than 1");
            }
        }else{
            std::__throw_runtime_error("Wrong ema config parameter");
        }
    }
    return config;
}
}
}

}