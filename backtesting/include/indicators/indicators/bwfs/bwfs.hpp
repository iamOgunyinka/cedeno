#ifndef BWFS_HPP_
#define BWFS_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "indc.hpp"

namespace indicators{
struct conf_BWFS_t;
struct indicator_t;


enum class bwfs_mode_e{
    STATIC,
    DYNAMIC,
};

struct conf_BWFS_t{
    bwfs_mode_e mode = bwfs_mode_e::STATIC;
    uint64_t time = 1;
    double client_confirmation = 0.0;
};

struct bwfs_t : public indcs_c{
    bwfs_t( indicators::indicator_t &common_db_, 
            indicators::conf_BWFS_t &configuration_);
    ~bwfs_t();
    indicators::indicator_t *common_db;
    indicators::conf_BWFS_t *configuration;
};
}
#include "indc_data.hpp"
namespace indicators{
namespace config{
namespace bwfs{
indicators::conf_BWFS_t get_config( const std::vector<std::string> &indcs,
                                         std::array<bool, (uint64_t)indicators::types_e::SIZE> *indc_states,
                                         uint64_t &counter);
}
}
}
#endif