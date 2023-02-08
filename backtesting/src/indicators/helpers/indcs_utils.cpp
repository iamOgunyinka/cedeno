#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{
namespace indcs_utils{

std::pair<std::string, std::string> split_string(const std::string &str, const std::string &separator){
    size_t colon_idx = str.find(separator);
    if(colon_idx == std::string::npos)
        std::__throw_runtime_error("Wrong indicator or config structure");
    return std::pair<std::string, std::string>(
            str.substr(0,colon_idx),
            str.substr( colon_idx + 1, str.size() - (colon_idx + 1))
    );
}

}//INDCS_UTILS
}//INDICATORS