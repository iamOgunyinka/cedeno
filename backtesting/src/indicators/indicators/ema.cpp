#include "indicators/ema.hpp"
#include "indicators/helpers/indcs_utils.hpp"


namespace indicators{

static void ema_calculate( ema_t &handler, 
                           const double &curr_price){
    double k = 2/(handler.configuration->n + 1);
    double &ema_price = handler.common_db->indc_info.ema.price;
    ema_price = k*(curr_price - ema_price) + ema_price;
}

static bool sma_calculate( ema_t &handler,
                           const kline_test_t &kline_data){
    handler.SMA.sumatory += kline_data.price;
    if(++handler.SMA.counter == handler.configuration->n){
        handler.common_db->indc_info.ema.price = handler.SMA.sumatory/handler.SMA.counter;
        return true;
    }
    return false;
}

void ema_callback( const kline_test_t &kline_data, 
                   indicator_t &handler_){
    ema_t &handler = *handler_.indcs_var.ema_vars;
    std::cout<<__func__<<std::endl;
    if(handler.SMA.calculating == true){
        if(sma_calculate(handler, kline_data))
            handler.SMA.calculating = false;
    }else{
        ema_calculate(handler, kline_data.price);
    }    
}

namespace config{
namespace ema{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_){
    conf_ema_t &config = *((conf_ema_t*)(config_));
    config = conf_ema_t();
    if(indcs.size() > 1){
        auto config_pair = utils::split_string(indcs[1], ":");
        if(config_pair.first == "n"){
            if(!utils::check_if_string_is_number(config_pair.second)){
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
    (*indc_states)[(uint64_t)types_e::EMA] = true;
    types_counter[(uint64_t)source_e::SRC_KLINE]++;
}

}
}

}