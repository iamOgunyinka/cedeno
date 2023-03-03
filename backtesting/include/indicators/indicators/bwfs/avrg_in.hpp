#ifndef AVRG_IN_HPP_
#define AVRG_IN_HPP_

#include "indc_data.hpp"
#include "indicators/bwfs/bwfs.hpp"

namespace indicators{

struct avrg_in_t: public bwfs_t{
    avrg_in_t( indicator_t &common_db_, 
               conf_BWFS_t &configuration_):
        bwfs_t( common_db_, configuration_){}

    ~avrg_in_t(){}
};

void avrg_in_callback( const trade_stream_d &trade_data, 
                       indicator_t &handler_);

}
#endif