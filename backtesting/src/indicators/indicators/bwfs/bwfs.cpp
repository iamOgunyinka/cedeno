#include "indicators/bwfs/bwfs.hpp"

#include <algorithm>

#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{

namespace bwfs{
    
static void check_indc_confg_params_( const std::vector<std::string> &indcs, 
                                      const uint64_t &config_idx, 
                                      const uint64_t &max_params_sz, 
                                      const std::string &indc_type){
    int item_left_in_vector = indcs.size() - config_idx;
    if(item_left_in_vector > max_params_sz){
        const std::string error_message =   "So many " + 
                                            indc_type + 
                                            " config indicator parameters"; 
        std::__throw_runtime_error(error_message.c_str());
    }
}

static void get_indicators_( const std::vector<std::string> &indicators, 
                             std::array<bool, (uint64_t)inds_e::SIZE> &indc_states,
                             uint64_t &trade_sz, 
                             uint8_t &config_idx){
    config_idx = 0;
    for(auto &itr: indicators){
        auto item = indc_list.find(itr);
        if( item != indc_list.end()){
            indc_states[item->second] = true;
            config_idx++;
            trade_sz++;
        }
    }
}

static void get_config_( const std::vector<std::string> &indcs,
                         indicators::ind_BWFS_confg_t &config,  
                         const uint64_t &indx){
    
    std::for_each(indcs.begin() + indx, indcs.end(), [&](const std::string &config_str){
            auto config_pair = indicators::indcs_utils::split_string(config_str, ":"); 
            if(config_pair.first == "mode"){
                if(config_pair.second == "static"){
                    config.mode = ind_mode_e::STATIC;
                }else if(config_pair.second == "dynamic"){
                    config.mode = ind_mode_e::DYNAMIC;
                }else{
                    std::__throw_runtime_error("Wrong BWFS-mode");
                }
            }else if(config_pair.first == "time"){
                config.time = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else if(config_pair.first == "client_confirmation"){
                config.client_confirmation = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else{
                std::__throw_runtime_error("Wrong BWFS config parameter");
            }
        }
    );
}

indicators::ind_BWFS_confg_t get_config( const std::vector<std::string> &indcs,
                                         std::array<bool, (uint64_t)inds_e::SIZE> &indc_states,
                                         uint64_t &counter){
    indicators::ind_BWFS_confg_t config;
    uint8_t config_idx = 0;

    get_indicators_(indcs, indc_states, counter, config_idx);
    if((config_idx + 1) > indcs.size()){
        config = ind_BWFS_confg_t(); 
    }else{
        check_indc_confg_params_(indcs, config_idx, 3, "BWFS");
        get_config_(indcs, config, config_idx);
    }

    std::cout<<"BWFS Config: "<<std::endl<<"time: "<<config.time<<std::endl; 
    std::cout<<"mode: "<<(int)config.mode<<std::endl; 
    std::cout<<"limit: "<<config.client_confirmation<<std::endl;
    return config;
}

}
}