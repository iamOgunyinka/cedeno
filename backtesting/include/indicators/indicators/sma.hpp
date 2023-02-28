#ifndef SMA_HPP_
#define SMA_HPP_

#include <queue>

#include "indc_data.hpp"

#include "source_data.hpp"

namespace indicators{

struct conf_sma_t{
    uint64_t n = 1;
};

struct sma_t{
    sma_t( indicator_t &common_db_,
           conf_sma_t &configuration_):
        common_db(&common_db_),
        configuration(&configuration_){}

    ~sma_t(){}

    indicator_t *common_db;
    conf_sma_t *configuration;
    double sumatory = 0.0;
    uint64_t n = 0;
    std::queue<double> prices_q;
};

void sma_callback( const kline_d &kline_data, 
                   indicator_t &handler_);

namespace config{
namespace sma{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_,
                std::vector<source_e> &sources);
}
}
}
#endif
