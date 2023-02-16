#ifndef SMA_HPP_
#define SMA_HPP_

#include "user_data.hpp"
#include "indc_data.hpp"

#include "testing.hpp"

namespace indicators{
struct sma_t{
    sma_t( indicators::indicator_t &common_db_):
           common_db(&common_db_){}
    ~sma_t(){}
    indicators::indicator_t *common_db;
    double sumatory = 0.0;
    uint8_t n = 0;
};

void sma_callback( const kline_test_t &kline_data, 
                   indicator_t &handler_);
}
#endif
