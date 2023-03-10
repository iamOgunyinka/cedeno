#ifndef INDCS_UTILS_HPP_
#define INDCS_UTILS_HPP_
#include <iostream>
#include <string>
#include <vector>

namespace indicators{
namespace utils{
enum num_types{
    NO_NUMBER,
    DOUBLE,
    INTEGER,
};

std::pair<std::string, std::string> split_string( const std::string &config, 
                                                  const std::string &separator);

num_types check_if_string_is_valid_number(const std::string &str);

void split_string_by_delimiter( const std::string &str, 
                                const char &delimiter,
                                std::vector<std::string> &result);
}
}
#endif