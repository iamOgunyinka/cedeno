#include "config.hpp"

namespace indicators{
namespace config{


config_pair_t  get_number( const std::string &str){
    config_pair_t result;
    auto config_pair = utils::split_string(str, ":");

    // std::cout<<"one";
    result.first = config_pair.first;
    // std::cout<<"two";
    // utils::num_types num_type = utils::get_number_type_from_string(config_pair.second);
    // if(num_type != utils::num_types::NO_NUMBER){
    //     if(num_type != utils::num_types::DOUBLE){
    //         std::cout<<"double";
    //         result.second = std::stod(config_pair.second);
    //     }
    //     else{
    //         std::cout<<"uin64";
    //         result.second = (uint64_t)strtoul(config_pair.second.c_str(), nullptr, 10);
    //     }
    // }else{
    //     std::cout<<"string";
    //     result.second = config_pair.second;
    // }
    return result;
}

}
}