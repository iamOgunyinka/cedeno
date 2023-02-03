#include <iostream>
#include "indicators.hpp"
#include <chrono>
#include <vector>
#include <string>

void enable_all_indicators(indicators::indicators_c &indicators){
}

int main(void){
    indicators::indicators_c indcs;
    std::vector<std::vector<std::string>> indcs_confg{
        {"qty_in","qty_out","tick_in","tick_out","avrg_out","avrg_in","tick_in_out","qty_in_out","static", "2", "3"},
    };
    indcs.set(indcs_confg);
    backtesting::trade_list_t trade_list;
    backtesting::trade_data_t trade_data; 
    trade_data.tokenName = "hello";
    trade_data.eventTime =  std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    trade_list.push_back(trade_data);
    indcs.process(trade_list);
    return 0;
}
