#include "ticks_in.hpp"
#include "indi_data.hpp"

namespace indicators{

static void ticks_in_update(indictors_t &indicator, const backtesting::trade_data_t &trade_data){
    if(trade_data.side == backtesting::trade_side_e::buy){
        indicator.ticks_in++;
    }
}

void ticks_in(const backtesting::trade_data_t &trade_data){ 
    auto it = data_base.find(trade_data.tokenName);
    if(it != data_base.end()){
        ticks_in_update(it->second, trade_data);
    }else{
        indictors_t indicator;
        ticks_in_update(indicator, trade_data);
        data_base[trade_data.tokenName] = indicator;
    }
}

}