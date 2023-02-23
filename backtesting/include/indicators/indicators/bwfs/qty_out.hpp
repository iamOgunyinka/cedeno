#ifndef QTY_OUT_HPP_
#define QTY_OUT_HPP_

#include "indc_data.hpp"
#include "user_data.hpp"
#include "indicators/bwfs/bwfs.hpp"

namespace indicators{

struct qty_out_t: public bwfs_t{
    qty_out_t( indicator_t &common_db_, 
               conf_BWFS_t &configuration_):
        bwfs_t( common_db_, configuration_){}

    ~qty_out_t(){}
};
 
void qty_out_callback( const backtesting::trade_data_t &trade_data, 
                       indicator_t &handler_);

}

#endif