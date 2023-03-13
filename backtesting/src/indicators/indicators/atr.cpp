#include "indicators/atr.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
static double atr_get_max_value( const kline_d &kline_data,
                                 atr_t &handler){
    double H_L = kline_data.highPrice - kline_data.lowPrice;
    double H_Cp = std::abs(kline_data.highPrice - handler.last_price);
    double L_Cp = std::abs(kline_data.lowPrice - handler.last_price);

    if(H_L >= H_Cp && H_L >= L_Cp){
        return H_L;
    }else if(H_Cp >= H_L && H_Cp >= L_Cp){
        return H_Cp;
    }else{
        return L_Cp;
    }
    return 0.0;
}

static void atr_calculate(const kline_d &kline_data, 
                          atr_t &handler){

    uint64_t &n = handler.configuration->n;

    double total_sum;
    for(auto &price : handler.prices_q){
        total_sum += price; 
    }

    double atr_prev = total_sum/n;
    double atr_curr = atr_get_max_value(kline_data, handler);
    handler.common_db->indc_info.atr.price = ((atr_prev*(n - 1)) + atr_curr) / n; 

    handler.prices_q.pop_back();
    handler.prices_q.push_back(atr_curr);
}

void atr_callback( const kline_d &kline_data, 
                   indicator_t &handler_){
    atr_t &handler = *handler_.indcs_var.atr_vars;
    std::cout<<__func__<<std::endl;
    if(handler.n == handler.configuration->threshold){
        atr_calculate(kline_data, handler);
    }else{
        if(handler.n >= 1){
            handler.prices_q.push_back(atr_get_max_value(kline_data, handler));
        }
        handler.n++;
    }
    handler.last_price = kline_data.closePrice;

}

namespace config{
namespace atr{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources){
    conf_atr_t &config = *((conf_atr_t*)(config_));
    config = conf_atr_t();
    if(indcs.size() > 1){
        auto config_pair = utils::split_string(indcs[1], ":");
        if(config_pair.first == "n"){
            if(!(bool)utils::check_if_string_is_valid_number(config_pair.second)){
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
    (*indc_states)[(uint64_t)types_e::ATR] = true;
    types_counter[(uint64_t)source_e::SRC_KLINE]++;
    config.threshold = config.n + 1;
}
}
}
}