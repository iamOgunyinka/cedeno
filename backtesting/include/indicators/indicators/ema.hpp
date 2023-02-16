#ifndef EMA_HPP_
#define EMA_HPP_

#include "user_data.hpp"
#include "indc_data.hpp"

namespace indicators{

struct conf_ema_t{
    uint64_t n = 2;
};

struct ema_t{
    ema_t( indicators::indicator_t &common_db_,
           conf_ema_t &configuration_):
           common_db(&common_db_),
           configuration(&configuration_){}
    ~ema_t(){}
    indicators::indicator_t *common_db;
    const conf_ema_t *configuration;

    struct{
        bool calculating = true;
        double sumatory = 0.0;
        uint8_t counter = 0;
    }SMA;
};

void ema_callback( const backtesting::trade_data_t &trade_data, 
                   indicator_t &handler_);

namespace config{
namespace ema{
conf_ema_t get_config(const std::vector<std::string> &indcs);
}
}

}
#endif
