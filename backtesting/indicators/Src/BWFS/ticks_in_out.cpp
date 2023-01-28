#include "BWFS/ticks_in_out.hpp"

namespace indicators{

void ticks_in_out( indicators::indicators_list_t &ind_list, indicator_data_t &new_data, const backtesting::trade_data_t &trade_data){
    std::cout<<__func__<<std::endl;
    new_data.cab.ticks_in_out = new_data.cab.ticks_in - new_data.cab.ticks_out;
}

}