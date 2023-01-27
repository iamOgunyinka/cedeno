#ifndef QTY_OUT_HPP_
#define QTY_OUT_HPP_
#include "user_data.hpp"
#include "indicator_data.hpp"

namespace indicators{

void qty_out( indicators::indicators_list_t &ind_list, indicator_data_t &new_data, const backtesting::trade_data_t &trade_data);
        
}

#endif