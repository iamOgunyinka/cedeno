#ifndef BWFS_HPP_
#define BWFS_HPP_

#include "indc_data.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

namespace indicators{
namespace bwfs{
indicators::ind_BWFS_confg_t get_config( const std::vector<std::string> &indcs,
                                         std::array<bool, (uint64_t)inds_e::SIZE> &indc_states,
                                         uint64_t &counter);

}
}
#endif