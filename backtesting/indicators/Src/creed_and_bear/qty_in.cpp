#include "creed_and_bear/qty_in.hpp"

namespace indicators{

void qty_in( indicators::indicators_list_t &ind_list, indicator_data_t &new_data, const backtesting::trade_data_t &trade_data){
    std::cout<<__func__<<std::endl;    
    if(trade_data.side == backtesting::trade_side_e::buy){
        new_data.cab.qty_in = ind_list.back().cab.qty_in;
        new_data.cab.qty_in+= trade_data.amountPerPiece;
    }
}

}