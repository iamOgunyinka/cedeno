#include "indicators/helpers/indcs_utils.hpp"
#include <algorithm>
namespace indicators{
namespace utils{

std::pair<std::string, std::string> split_string( const std::string &str, 
                                                  const std::string &separator){
    size_t colon_idx = str.find(separator);
    
    if(colon_idx == std::string::npos)
        std::__throw_runtime_error("Wrong config structure");

    return std::pair<std::string, std::string>(
            str.substr(0,colon_idx),
            str.substr( colon_idx + 1, str.size() - (colon_idx + 1))
    );
}

bool check_if_string_is_number(const std::string &str){
    size_t sz = str.size();
    if(sz == 1){
        return std::isdigit(str[0]);
    }

    for(uint64_t idx = 0, counter = 0; idx < sz; idx++){
        const char &c = str[idx];
        if(c == '.'){
            if(idx == 0 || idx == (sz-1))
                return false;
            if(++counter == 2)
                return false;
        }else if(!std::isdigit(c)){
            return false;
        }
    }
    return true;
}

void split_string_by_delimiter( const std::string &str, 
                                const char &delimiter,
                                std::vector<std::string> &result){
    size_t end_index = 0;
    size_t start_index = 0;
    while (end_index != std::string::npos)
    {   
        end_index = str.find_first_of(delimiter,start_index);
        result.push_back(str.substr(start_index, end_index - start_index));
        start_index = end_index+1;
    }
}

}//INDCS_UTILS
}//INDICATORS