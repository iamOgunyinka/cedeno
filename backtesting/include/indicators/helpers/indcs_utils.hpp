#ifndef INDCS_UTILS_HPP_
#define INDCS_UTILS_HPP_
#include <iostream>
#include <string>

namespace indicators{
namespace indcs_utils{
std::pair<std::string, std::string> split_string(const std::string &config, const std::string &separator);
}
}
#endif