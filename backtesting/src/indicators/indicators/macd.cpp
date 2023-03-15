#include "indicators/macd.hpp"
#include "indicators/helpers/indcs_utils.hpp"

#include <algorithm>

namespace indicators{

static void calculate_macd(macd_t &handler){
    inf_macd_t &inf_macd = handler.common_db->info.macd;
    inf_macd.price = handler.ema_q[handler.configuration->high_period] - 
                     handler.ema_q[handler.configuration->low_period];
}

void macd_callback( const kline_d &kline_data, 
                    indicator_t &handler_){
    macd_t &handler = *handler_.indcs_var.macd_vars;
    std::cout<<__func__<<std::endl;
    if(handler.n <= handler.configuration->high_period){
        handler.ema_q.push_front(handler.common_db->info.ema.price);
        if(handler.n == handler.configuration->high_period){
            calculate_macd(handler);
        }
        handler.n++;
    }else{
        handler.ema_q.pop_back();
        handler.ema_q.push_front(handler.common_db->info.ema.price);
        calculate_macd(handler);
    }
}

namespace config{
namespace macd{

static void check_config_values_parameters(const conf_macd_t &config){
    if(config.high_period < config.low_period)
        std::__throw_runtime_error("MACD high period must be higher than low period");
    if( config.high_period == 0 || config.low_period == 0)
        std::__throw_runtime_error("MACD numbers params must be greater than 0");
        
}

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources){
    conf_macd_t &config = *((conf_macd_t*)config_);
    config = conf_macd_t();
    if(indcs.size() > 1){
        if(indcs.size() != 3)
            std::__throw_runtime_error("MACD incorrect numbers of config parameters");

        std::for_each(indcs.begin() + 1, indcs.end(), [&](const std::string &str){
            auto config_pair = utils::split_string(str, ":");

            if(!(bool)utils::check_if_string_is_valid_number(config_pair.second))
                std::__throw_runtime_error("Wrong ema config, n must be a number");

            if(config_pair.first == "high"){
                config.high_period = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else if(config_pair.first == "low"){
                config.low_period = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else{
                std::__throw_runtime_error("Wrong ema config parameter");
            }
        });
        check_config_values_parameters(config);
    }
    config.high_period--;
    config.low_period--;
    (*indc_states)[(uint64_t)types_e::MACD] = true;
    types_counter[(uint64_t)source_e::SRC_KLINE]++;
}

}
}

}