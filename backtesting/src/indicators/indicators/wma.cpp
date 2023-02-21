#include "indicators/wma.hpp"
#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
void wma_callback( const kline_test_t &kline_data, 
                   indicators::indicator_t &handler_){
    indicators::wma_t &handler = *handler_.indcs_var.wma_vars;
    std::cout<<__func__<<std::endl;
}

namespace config{
namespace wma{
static uint64_t get_n_parameter(const std::string &config){
    auto config_pair = indicators::indcs_utils::split_string(config, ":");
    uint64_t n = 0;
    if(config_pair.first == "n"){
        if(!indicators::indcs_utils::check_if_string_is_number(config_pair.second)){
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

static void get_w_parameter( const std::string &config, 
                             std::vector<double> &result){
    auto config_pair = indicators::indcs_utils::split_string(config, ":");
    if(config_pair.first == "w"){   
        std::vector<std::string> config_split;
        indicators::indcs_utils::split_string_by_delimiter(config_pair.second, ',', config_split);
        for(auto &num_str : config_split){
            if(!indicators::indcs_utils::check_if_string_is_number(num_str))
                std::__throw_runtime_error("Wrong wma config, w config must be only numbers");
            result.push_back(std::stod(num_str));
        }
    }else{
        std::__throw_runtime_error("Wrong wma config parameter");
    }
}

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)indicators::types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)data_types::SIZE> &types_counter,
                void *config_){
    conf_wma_t &config = *((conf_wma_t*)(config_));
    config = conf_wma_t();
    if(indcs.size() > 1){
        if(indcs.size() != 3)
            std::__throw_runtime_error("It is mandatory to pass 2 config parameters for wma indicator");

        config.n = get_n_parameter(indcs[1]);
        get_w_parameter(indcs[2], config.w);

        if(config.n != config.w.size()) 
            std::__throw_runtime_error("wrong wma config, n must be equal than w size");
    }
    (*indc_states)[(uint64_t)types_e::WMA] = true;
    types_counter[(uint64_t)indicators::data_types::INDC_KLINE]++;
}

}
}
}