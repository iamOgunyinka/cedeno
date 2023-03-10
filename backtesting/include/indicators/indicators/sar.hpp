#ifndef SAR_HPP_
#define SAR_HPP_

#include "indc_data.hpp"

#include "source_data.hpp"
namespace indicators{

struct conf_sar_t{
    double alpha = 2;
    uint64_t EMA = 99;
};

struct sar_t{
    sar_t( indicator_t &common_db_,
           conf_sar_t &configuration_):
        common_db(&common_db_),
        configuration(&configuration_){}

    ~sar_t(){}

    indicator_t *common_db;
    const conf_sar_t *configuration;

    struct{
        bool calculating = true;
        double sumatory = 0.0;
        uint8_t counter = 0;
    }SMA;
};

void sar_callback( const kline_d &kline_data, 
                   indicator_t &handler_);

namespace config{
namespace sar{

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_, 
                std::vector<source_e> &sources);
}
}

}
#endif
