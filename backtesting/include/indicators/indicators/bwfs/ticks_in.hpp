#ifndef TICKS_IN_HPP_
#define TICKS_IN_HPP_

#include "user_data.hpp"
// #include "indc_data.hpp"
#include "indicators/bwfs/bwfs.hpp"
namespace indicators{
struct ticks_in_t: public bwfs_t{   
    ticks_in_t( indicator_t &common_db_, 
                conf_BWFS_t &configuration_):
        bwfs_t( common_db_, configuration_){}

    ~ticks_in_t(){}
};

void ticks_in_callback( const backtesting::trade_data_t &trade_data, 
                        indicator_t &handler_);

}
#endif