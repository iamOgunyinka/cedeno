#include "BWFS/ticks_out.hpp"

namespace indicators{

void ticks_out( indicators::indicators_list_t &ind_list, indicator_data_t &new_data, const backtesting::trade_data_t &trade_data){
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::sell){
        new_data.cab.ticks_out = ind_list.back().cab.ticks_out;
        new_data.cab.ticks_out++;
    }
}

}