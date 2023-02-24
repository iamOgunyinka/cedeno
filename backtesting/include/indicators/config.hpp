#ifndef CONFIG_HPP_
#define CONFIG_HPP_
#include <string>
#include "helpers/indcs_utils.hpp"
#include <variant>

namespace indicators{
namespace config{
using config_pair_t = std::pair< std::string, 
        std::variant<double, uint64_t, int64_t, std::string>>; 
        
config_pair_t  get_number( const std::string &str);
}
}
#endif