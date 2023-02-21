#ifndef WMA_HPP_
#define WMA_HPP_

#include <queue>
#include <vector>

#include "user_data.hpp"
#include "indc_data.hpp"

#include "testing.hpp"

namespace indicators{

struct conf_wma_t{
    uint64_t n = 1;
    std::vector<double> w;
};

struct wma_t{
    wma_t( indicators::indicator_t &common_db_,
           indicators::conf_wma_t &configuration_):
           common_db(&common_db_),
           configuration(&configuration_){}
    ~wma_t(){}
    indicators::indicator_t *common_db;
    indicators::conf_wma_t *configuration;
    double sumatory = 0.0;
    uint64_t n = 0;
    std::queue<double> prices_q;
};

void wma_callback( const kline_test_t &kline_data, 
                   indicator_t &handler_);

namespace config{
namespace wma{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)indicators::types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)data_types::SIZE> &types_counter,
                void *config_);
}
}
}
#endif
