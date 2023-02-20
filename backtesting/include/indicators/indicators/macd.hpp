#ifndef MACD_HPP_
#define MACD_HPP_

#include <deque>

#include "user_data.hpp"
#include "indc_data.hpp"

#include "testing.hpp"

namespace indicators{

struct conf_macd_t{
    uint64_t high_period = 26;
    uint64_t low_period = 12;
};

struct macd_t{
    macd_t( indicators::indicator_t &common_db_,
           conf_macd_t &configuration_):
           common_db(&common_db_),
           configuration(&configuration_){}
    ~macd_t(){}
    indicators::indicator_t *common_db;
    const conf_macd_t *configuration;
    std::deque<double> ema_q;
    uint64_t n = 0;
};

void macd_callback( const kline_test_t &kline_data, 
                    indicator_t &handler_);

namespace config{
namespace macd{
void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)indicators::types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)data_types::SIZE> &types_counter,
                void *config_);
}
}
}

#endif
