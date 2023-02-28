#include "indicators/ema.hpp"
#include "indicators/helpers/indcs_utils.hpp"

#include <algorithm>


namespace indicators{

static void ema_calculate( ema_t &handler, 
                           const double &curr_price){
    double k = 2/(handler.configuration->n + 1);
    double &ema_price = handler.common_db->indc_info.ema.price;
    ema_price = k*(curr_price - ema_price) + ema_price;
}

static bool sma_calculate( ema_t &handler,
                           const double &price){
    handler.SMA.sumatory += price;
    if(++handler.SMA.counter == handler.configuration->n){
        handler.common_db->indc_info.ema.price = handler.SMA.sumatory/handler.SMA.counter;
        return true;
    }
    return false;
}

static void ema_callback(ema_t &handler,
                         const double &price){
    if(handler.SMA.calculating == true){
        if(sma_calculate(handler, price))
            handler.SMA.calculating = false;
    }else{
        ema_calculate(handler, price);
    }    
}

void ema_trade_callback( const trade_stream_d &trade_data, 
                         indicator_t &handler_){
    std::cout<<__func__<<std::endl;
    ema_callback(*handler_.indcs_var.ema_vars, trade_data.amountPerPiece);
}

void ema_kline_callback( const kline_d &kline_data, 
                         indicator_t &handler_){
    std::cout<<__func__<<std::endl;
    ema_callback(*handler_.indcs_var.ema_vars, kline_data.closePrice);
}

namespace config{
namespace ema{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_, 
                std::vector<source_e> &sources){
    conf_ema_t &config = *((conf_ema_t*)(config_));
    config = conf_ema_t();
    source_e src = source_e::SRC_KLINE;
    if(indcs.size() > 1){
        std::for_each(indcs.begin() + 1, indcs.end(), [&](const std::string &str){
            // config_pair_t result = get_number(str);
            auto config_pair = utils::split_string(str, ":");
            if(config_pair.first == "n"){
                if(!(bool)utils::get_number_type_from_string(config_pair.second)){ 
                    std::__throw_runtime_error("Wrong ema config, n must be a number");
                }
                config.n = strtoul(config_pair.second.c_str(), nullptr, 10);
                if(config.n < 1){
                    std::__throw_runtime_error("Wrong ema config, n must be greater than 1");
                }
            }else 
            if(config_pair.first == "source"){
                if(config_pair.second == "trade"){
                    src = source_e::SRC_TRADE;
                }else if(config_pair.second != "kline"){
                    std::__throw_runtime_error("Wrong ema config, wrong source config");
                }
            }else{
                std::__throw_runtime_error("Wrong ema config parameter");
            }
        });
    }
    (*indc_states)[(uint64_t)types_e::EMA] = true;
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