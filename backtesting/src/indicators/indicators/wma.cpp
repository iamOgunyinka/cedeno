#include "indicators/wma.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
static void wma_calculate(wma_t &handler){
    const std::size_t &sz = handler.prices_q.size();
    double &wma_price = handler.common_db->indc_info.macd.price;
    for(uint64_t idx = 0; idx < sz; idx++){
        wma_price += handler.prices_q[idx] *
                     handler.configuration->wi[idx]; 
    }
}

void wma_callback( const kline_d &kline_data, 
                   indicator_t &handler_){
    wma_t &handler = *handler_.indcs_var.wma_vars;
    std::cout<<__func__<<std::endl;
    if(handler.n < handler.configuration->n){
        handler.n++;
        handler.prices_q.push_front(kline_data.closePrice);
        if(handler.n == handler.configuration->n){
            wma_calculate(handler);
        }
    }else{
        handler.prices_q.pop_back();
        handler.prices_q.push_front(kline_data.closePrice);
        wma_calculate(handler);
    }
}

namespace config{
namespace wma{
static uint64_t get_n_parameter(const std::string &config){
    auto config_pair = utils::split_string(config, ":");
    uint64_t n = 0;
    if(config_pair.first == "n"){
        if(!(bool)utils::get_number_type_from_string(config_pair.second)){
            std::__throw_runtime_error("Wrong wma config, n must be a number");
        }
        n = strtoul(config_pair.second.c_str(), nullptr, 10);
        if(n < 1){
            std::__throw_runtime_error("Wrong wma config, n must be greater than 1");
        }
    }else{
        std::__throw_runtime_error("Wrong wma config parameter");
    }
    return n;
}

static void get_wi_parameter( const std::string &config, 
                             std::vector<double> &result){
    auto config_pair = utils::split_string(config, ":");
    if(config_pair.first == "w"){   
        std::vector<std::string> config_split;
        utils::split_string_by_delimiter(config_pair.second, ',', config_split);
        for(auto &num_str : config_split){
            if(!(bool)utils::get_number_type_from_string(num_str))
                std::__throw_runtime_error("Wrong wma config, w config must be only numbers");
            result.push_back(std::stod(num_str));
        }
    }else{
        std::__throw_runtime_error("Wrong wma config parameter");
    }
}

static void set_wi_consts(std::vector<double> wi){
    double w;
    for(auto &num : wi){
        w += num;
    }
    for(auto &num : wi){
        num = num/w;
    }
}

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources){
    conf_wma_t &config = *((conf_wma_t*)(config_));
    config = conf_wma_t();
    if(indcs.size() > 1){
        if(indcs.size() != 3)
            std::__throw_runtime_error("It is mandatory to pass 2 config parameters for wma indicator");

        config.n = get_n_parameter(indcs[1]);
        get_wi_parameter(indcs[2], config.wi);

        if(config.n != config.wi.size()) 
            std::__throw_runtime_error("wrong wma config, n must be equal than w size");

        set_wi_consts(config.wi);
    }   
    (*indc_states)[(uint64_t)types_e::WMA] = true;
    types_counter[(uint64_t)source_e::SRC_KLINE]++;
}

}
}
}