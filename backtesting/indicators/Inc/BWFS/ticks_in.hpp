#ifndef TICKS_IN_HPP_
#define TICKS_IN_HPP_
#include "user_data.hpp"
#include "indicator_data.hpp"

namespace indicators{

void ticks_in( indicators::indicators_list_t &ind_list, indicator_data_t &new_data, const backtesting::trade_data_t &trade_data);
        
}

#endif