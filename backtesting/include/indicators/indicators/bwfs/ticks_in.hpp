#ifndef TICKS_IN_HPP_
#define TICKS_IN_HPP_

#include "indicators/bwfs/bwfs.hpp"
namespace indicators{
struct ticks_in_t: public bwfs_t{   
    ticks_in_t( indicator_t &common_db_, 
                conf_BWFS_t &configuration_):
        bwfs_t( common_db_, configuration_){}

    ~ticks_in_t(){}
};

void ticks_in_callback( const trade_stream_d &trade_data, 
                        indicator_t &handler_);

}
#endif