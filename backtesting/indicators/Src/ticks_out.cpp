#include "ticks_out.hpp"
#include "indi_data.hpp"

namespace indicators{

static void ticks_out_update(indictors_t &indicator, const backtesting::trade_data_t &trade_data){
    if(trade_data.side == backtesting::trade_side_e::sell){
        indicator.ticks_out++;
    }
}

void ticks_out(const backtesting::trade_data_t &trade_data){ 
    auto it = data_base.find(trade_data.tokenName);
    if(it != data_base.end()){
        ticks_out_update(it->second, trade_data);
    }else{
        indictors_t indicator;
        ticks_out_update(indicator, trade_data);
        data_base[trade_data.tokenName] = indicator;
    }
}

}