#ifndef ATR_HPP_
#define ATR_HPP_

#include <queue>

#include "user_data.hpp"
#include "indc_data.hpp"

#include "testing.hpp"

namespace indicators{

struct conf_atr_t{
    uint64_t n = 14;
    uint64_t threshold = n+1;
};

struct atr_t{
    atr_t( indicator_t &common_db_,
           conf_atr_t &configuration_):
        common_db(&common_db_),
        configuration(&configuration_){}

    ~atr_t(){}

    indicator_t *common_db;
    conf_atr_t *configuration;
    double sumatory = 0.0;
    uint64_t n = 0;
    std::deque<double> prices_q;
    double last_price = 0.0;
};

void atr_callback( const kline_test_t &kline_data, 
                   indicator_t &handler_);

namespace config{
namespace atr{
void get_config( const std::vector<std::string> &indcs,
                 std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                 std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                 void *config_,
                 std::vector<source_e> &sources);
}
}
}
#endif
