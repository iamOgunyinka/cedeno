#include "BWFS/avrg_in.hpp"

namespace indicators{


void avrg_in( indicators::indicators_list_t &ind_list, indicator_data_t &new_data, const backtesting::trade_data_t &trade_data){
    std::cout<<__func__<<std::endl;
    if(trade_data.side == backtesting::trade_side_e::buy){
        new_data.cab.avrg_in = new_data.cab.qty_in/new_data.cab.ticks_in;
    }
}

}