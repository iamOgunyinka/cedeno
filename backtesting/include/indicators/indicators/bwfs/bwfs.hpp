#ifndef BWFS_HPP_
#define BWFS_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "indicators/indicator.hpp"

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
    bwfs_t( indicator_t &common_db_, 
            conf_BWFS_t &configuration_);

    ~bwfs_t();

    indicator_t *common_db;
    conf_BWFS_t *configuration;
};
}
#include "indc_data.hpp"
namespace indicators{
namespace config{
namespace bwfs{

void get_config( const std::vector<std::string> &,
                 std::array<bool, (uint64_t)types_e::SIZE>*,
                 std::array<uint64_t, (uint64_t)source_e::SIZE> &,
                 void *config_, 
                 std::vector<source_e> &sources);
                 
}
}
}
#endif