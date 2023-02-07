#include "indicators.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <string>

indicators::indicators_c indcs;
void process_data(indicators::indicators_c &indcs){
    backtesting::trade_list_t trade_list;
    backtesting::trade_data_t trade_data; 
    trade_data.tokenName = "hello";
    trade_data.side = backtesting::trade_side_e::buy;
    trade_data.eventTime =  std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    trade_list.push_back(trade_data);
    indcs.process(trade_list);
}

int main(void){
    std::vector<std::vector<std::string>> indcs_confg{
        // {"qty_in","tick_out","tick_out","avrg_in","tick_in_out","static", "2", "3"},
        // {"qty_in","static", "2", "3"},
        // {"qty_in","mode:static", "2"},
        // {"qty_in","qty_out","tick_in","tick_out","avrg_out","avrg_in","tick_in_out","qty_in_out"},
        {"qty_in","qty_out","tick_in","tick_out","avrg_out","avrg_in","tick_in_out","qty_in_out", "limit:909","time:4"},
    };
    indcs.set(indcs_confg);
    process_data(indcs);
    return 0;
}
