#include "indicators/sar.hpp"
#include "indicators/helpers/indcs_utils.hpp"

#include <algorithm>


namespace indicators{

void sar_callback( const kline_d &kline_data, 
          indicator_t &handler_){
    sar_t &handler = *handler_.indcs_var.sar_vars;
    std::cout<<__func__<<std::endl;
}

namespace config{
namespace sar{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_, 
                std::vector<source_e> &sources){
    conf_sar_t &config = *((conf_sar_t*)(config_));
    config = conf_sar_t();
    source_e src = source_e::SRC_KLINE;
    if(indcs.size() > 1){
        std::for_each(indcs.begin() + 1, indcs.end(), [&](const std::string &str){

            auto config_pair = utils::split_string(str, ":");
            if(config_pair.first == "a"){
                if(!(bool)utils::check_if_string_is_valid_number(config_pair.second)){ 
                    std::__throw_runtime_error("Wrong sar config, n must be a number");
                }
                config.alpha = std::stod(config_pair.second.c_str());
            }else if(config_pair.first == "ema"){
                if(!(bool)utils::check_if_string_is_valid_number(config_pair.second)){ 
                    std::__throw_runtime_error("Wrong sar config, n must be a number");
                }
                config.EMA = std::strtoul(config_pair.second.c_str(), nullptr, 10);
            }else{
                if(config_pair.first == "source"){
                    if(config_pair.second == "trade"){
                        src = source_e::SRC_TRADE;
                    }else if(config_pair.second != "kline"){
                        std::__throw_runtime_error("Wrong sar config, wrong source config");
                    }
                }else{
                    std::__throw_runtime_error("Wrong sar config parameter");
                }
            } 
        });
    }

    (*indc_states)[(uint64_t)types_e::SAR] = true;
    if(src == source_e::SRC_KLINE){
        types_counter[(uint64_t)source_e::SRC_KLINE]++;
        sources = std::vector<source_e>({source_e::SRC_KLINE});
    }
    else{
        types_counter[(uint64_t)source_e::SRC_TRADE]++;
        sources = std::vector<source_e>({source_e::SRC_TRADE});
    }
}

}
}

}