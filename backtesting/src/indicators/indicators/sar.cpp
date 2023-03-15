#include "indicators/sar.hpp"
#include "indicators/helpers/indcs_utils.hpp"

#include <algorithm>


namespace indicators{

static void ema_calculate( sar_t &handler, 
                           const double &curr_price){
    double k = 2/(handler.configuration->EMA + 1);
    double &ema_price = handler.EMA.price;
    ema_price = k*(curr_price - ema_price) + ema_price;
}

static bool sma_calculate( sar_t &handler,
                           const double &curr_price){
    handler.EMA.SMA.sumatory += curr_price;
    if(++handler.EMA.SMA.counter == handler.configuration->EMA){
        handler.EMA.price = handler.EMA.SMA.sumatory/handler.configuration->EMA;
        return true;
    }
    return false;
}

static bool ema_handler(sar_t &handler,
                        const double &price){
    if(handler.EMA.SMA.calculating == true){
        if(sma_calculate(handler, price)){
            handler.EMA.SMA.calculating = false;
        }else{
            return false;
        }
    }else{
        ema_calculate(handler, price);
    }    
    return true;
}

static void sar_calculate( sar_t &handler,
                           const kline_d &kline_data){
    inf_sar_t &sar = handler.common_db->info.sar;

    if(kline_data.highPrice > handler.ep_uptrend)   
        handler.ep_uptrend = kline_data.highPrice;

    sar.price_up = sar.price_up + 
                    handler.configuration->alpha*( handler.ep_uptrend - sar.price_up);

    if(kline_data.lowPrice < handler.ep_downtrend)   
        handler.ep_downtrend = kline_data.lowPrice;

    sar.price_down = sar.price_down - 
                        handler.configuration->alpha*( sar.price_up - handler.ep_downtrend);

    if(handler.EMA.price > kline_data.closePrice)
        handler.common_db->info.sar.status = true;
    else
        handler.common_db->info.sar.status = false;
}

void sar_callback( const kline_d &kline_data, 
                   indicator_t &handler_){
    sar_t &handler = *handler_.indcs_var.sar_vars;
    std::cout<<__func__<<std::endl;
    if(ema_handler(handler, kline_data.closePrice)){
        sar_calculate(handler, kline_data);
    }
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