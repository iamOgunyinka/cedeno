#include "indicators/macd.hpp"
#include "indicators/helpers/indcs_utils.hpp"

#include <algorithm>

namespace indicators{

static void calculate_macd(indicators::macd_t &handler){
    indicators::inf_macd_t &inf_macd = handler.common_db->indc_info.macd;
    inf_macd.price = handler.ema_q[handler.configuration->high_period] - 
                     handler.ema_q[handler.configuration->low_period];
}

void macd_callback( const kline_test_t &kline_data, 
                    indicators::indicator_t &handler_){
    indicators::macd_t &handler = *handler_.indcs_var.macd_vars;
    std::cout<<__func__<<std::endl;
    if(handler.n <= handler.configuration->high_period){
        handler.ema_q.push_front(handler.common_db->indc_info.ema.price);
        if(handler.n == handler.configuration->high_period){
            calculate_macd(handler);
        }
        handler.n++;
    }else{
        handler.ema_q.pop_back();
        handler.ema_q.push_front(handler.common_db->indc_info.ema.price);
        calculate_macd(handler);
    }
}

namespace config{
namespace macd{

static void check_config_values_parameters(const indicators::conf_macd_t &config){
    if(config.high_period < config.low_period)
        std::__throw_runtime_error("MACD high period must be higher than low period");
    if( config.high_period == 0 || config.low_period == 0)
        std::__throw_runtime_error("MACD numbers params must be greater than 0");
        
}

indicators::conf_macd_t get_config(const std::vector<std::string> &indcs){
    indicators::conf_macd_t config;
    if(indcs.size() > 1){
        if(indcs.size() != 3)
            std::__throw_runtime_error("MACD incorrect numbers of config parameters");
        std::for_each(indcs.begin() + 1, indcs.end(), [&](const std::string &str){
            auto config_pair = indicators::indcs_utils::split_string(str, ":");

            if(!indicators::indcs_utils::check_if_string_is_number(config_pair.second))
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
    return config;
}
}
}

}