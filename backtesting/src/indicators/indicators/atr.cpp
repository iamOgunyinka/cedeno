#include "indicators/atr.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
void atr_callback( const kline_test_t &kline_data, 
                   indicator_t &handler_){
    atr_t &handler = *handler_.indcs_var.atr_vars;
    std::cout<<__func__<<std::endl;
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
            if(!(bool)utils::get_number_type_from_string(config_pair.second)){
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
}
}
}
}