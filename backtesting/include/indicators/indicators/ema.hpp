#ifndef EMA_HPP_
#define EMA_HPP_

#include "indc_data.hpp"

#include "source_data.hpp"
namespace indicators{

struct conf_ema_t{
    uint64_t n = 2;
};

struct ema_t{
    ema_t( indicator_t &common_db_,
           conf_ema_t &configuration_):
        common_db(&common_db_),
        configuration(&configuration_){}

    ~ema_t(){}

    indicator_t *common_db;
    const conf_ema_t *configuration;

    struct{
        bool calculating = true;
        double sumatory = 0.0;
        uint8_t counter = 0;
    }SMA;
};

void ema_trade_callback( const trade_stream_d &trade_data, 
                         indicator_t &handler_);
void ema_kline_callback( const kline_d &kline_data, 
                         indicator_t &handler_);

namespace config{
namespace ema{

void get_config( const std::vector<std::string> &indcs,
                std::array<bool, (uint64_t)types_e::SIZE> *indc_states,
                std::array<uint64_t, (uint64_t)source_e::SIZE> &types_counter,
                void *config_, 
                std::vector<source_e> &sources);
}
}

}
#endif
