#include "indicators/bwfs/bwfs.hpp"

#include <algorithm>

#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{    

bwfs_t::bwfs_t( indicator_t &common_db_, 
                conf_BWFS_t &configuration_): indcs_c(common_db_){
    common_db = &common_db_;
    configuration = &configuration_;
}

bwfs_t::~bwfs_t(){

}

namespace config{
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
        throw std::runtime_error(error_message.c_str());
    }
}

static void get_indicators_( const std::vector<std::string> &indicators, 
                             std::array<bool, (uint64_t)types_e::SIZE> &indc_states,
                             uint64_t &trade_sz, 
                             uint8_t &config_idx){
    std::for_each(indicators.begin(), indicators.end(), [&](const std::string indc){
        auto item = indc_list_key_string.find(indc);
        if( item != indc_list_key_string.end()){
            if(indc_states[item->second])
                throw std::runtime_error(std::string( "Setting " + item->first
                                                        + " twice").c_str());
            indc_states[item->second] = true;
            config_idx++;
            trade_sz++;
        }
    });
}

static void get_config_( const std::vector<std::string> &indcs,
                         conf_BWFS_t &config,  
                         const uint64_t &indx){
    
    std::for_each(indcs.begin() + indx, indcs.end(), [&](const std::string &config_str){
            auto config_pair = utils::split_string(config_str, ":"); 
            if(config_pair.first == "mode"){
                if(config_pair.second == "static"){
                    config.mode = bwfs_mode_e::STATIC;
                }else if(config_pair.second == "dynamic"){
                    config.mode = bwfs_mode_e::DYNAMIC;
                }else{
                    throw std::runtime_error("Wrong BWFS-mode");
                }
            }else if(config_pair.first == "time"){
                config.time = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else if(config_pair.first == "client_confirmation"){
                config.client_confirmation = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else{
                throw std::runtime_error("Wrong BWFS config parameter");
            }
        }
    );
}

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources){
    conf_BWFS_t &config = *((conf_BWFS_t*)config_);
    config = conf_BWFS_t(); 
    uint8_t config_idx = 0;

    get_indicators_(indcs, *indc_states, types_counter[(uint64_t)source_e::SRC_TRADE], config_idx );
    if(config_idx  < indcs.size()){
        check_indc_confg_params_(indcs, config_idx, 3, "BWFS");
        get_config_(indcs, config, config_idx);
    }
    std::cout<<"BWFS Config: "<<std::endl<<"time: "<<config.time<<std::endl; 
    std::cout<<"mode: "<<(int)config.mode<<std::endl; 
    std::cout<<"limit: "<<config.client_confirmation<<std::endl;
}
}

}

}