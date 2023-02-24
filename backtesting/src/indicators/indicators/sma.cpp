#include "indicators/sma.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
void sma_callback( const kline_test_t &kline_data, 
                   indicator_t &handler_){
    sma_t &handler = *handler_.indcs_var.sma_vars;
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

namespace config{
namespace sma{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources){
    conf_sma_t &config = *((conf_sma_t*)(config_));
    config = conf_sma_t();
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
    (*indc_states)[(uint64_t)types_e::SMA] = true;
    types_counter[(uint64_t)source_e::SRC_KLINE]++;
}
}
}
}