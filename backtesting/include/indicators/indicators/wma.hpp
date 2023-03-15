#ifndef WMA_HPP_
#define WMA_HPP_

#include <queue>
#include <vector>

#include "indc_data.hpp"

namespace indicators{

struct conf_wma_t{
    uint64_t n = 1;
    std::vector<double> wi;
};

struct wma_t{
    wma_t( indicator_t &common_db_,
           conf_wma_t &configuration_):
        common_db(&common_db_),
        configuration(&configuration_){}

    ~wma_t(){}

    indicator_t *common_db;
    conf_wma_t *configuration;
    double sumatory = 0.0;
    uint64_t n = 0;
    std::deque<double> prices_q;
};  

void wma_callback( const kline_d &kline_data, 
                   indicator_t &handler_);

namespace config{
namespace wma{

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources);

}
}
}
#endif
