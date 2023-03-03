#ifndef AVRG_OUT_HPP_
#define AVRG_OUT_HPP_

#include "indc_data.hpp"
#include "indicators/bwfs/bwfs.hpp"
namespace indicators{

struct avrg_out_t: public bwfs_t{
    avrg_out_t( indicator_t &common_db_, 
                conf_BWFS_t &configuration_):
        bwfs_t( common_db_, configuration_){}

    ~avrg_out_t(){}
};

void avrg_out_callback( const trade_stream_d &trade_data, 
                        indicator_t &handler_);

}

#endif