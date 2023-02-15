#include "indicators.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <array>

/*GLOBAL VAR*/
indicators::indicators_c indicator_handler;
using namespace backtesting;

/*=========================================================*/
/*======================PYTHON SCRIPT======================*/

void python_script(void){
    std::vector<std::vector<std::string>> indicator_config{
        // {"qty_in","qty_out","tick_in","tick_out"},
        // {"qty_out","ticks_in","qty_in","tick_out"}, /*diferent order*/
        // {"qty_in","qty_out","tick_in","tick_out", "avrg_out", "avrg_n", "qty_in_out", "tick_in_out"}, /*enable all*/
        // {"qty_in","qty_out","tick_in","tick_out", "avrg_out", "avrg_n", "mode:static"}, /*config mode*/
        {"qty_in","qty_out","tick_in","tick_out", "avrg_out", "avrg_in","buy_vs_sell", "mode:dynamic", "client_confirmation:878","time:45"}, /*config mode and time*/
        {"ema"},
        // {"qty_in","qty_out","tick_in","tick_out", "avrg_out", "avrg_in", "mode:dynamic", "tim:45"}, /*config wrong parameter*/
        // {"qty_in","qty_out","tick_in","tick_out", "avrg_out", "avrg_in", "mode:dynamic", "time:45", "limit:89"},/*config all including parameters*/
    };
    indicator_handler.set(indicator_config);
}
/*======================PYTHON SCRIPT======================*/
/*=========================================================*/



trade_list_t set_trade_list(indicators::indicators_c &indcs, double price, trade_side_e side){

    trade_list_t trade_list;
    trade_data_t trade_data; 
    trade_data.tokenName = "BTCUSDT";
    trade_data.amountPerPiece = price;
    trade_data.side = side;
    trade_data.eventTime =  std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    trade_list.push_back(trade_data);
    return trade_list;
}

void print_info(const std::string &symbol){
    indicators::inf_t indicator_info = indicator_handler.get(symbol);

    std::cout<<std::endl<<"INFO:"<<std::endl
             <<"ticks_in: "<<indicator_info.cab.ticks_in<<std::endl
             <<"ticks_out: "<<indicator_info.cab.ticks_out<<std::endl
             <<"qty_in: "<<indicator_info.cab.qty_in<<std::endl
             <<"qty_out: "<<indicator_info.cab.qty_out<<std::endl
             <<"avrg_in: "<<indicator_info.cab.avrg_in<<std::endl
             <<"avrg_out: "<<indicator_info.cab.avrg_out<<std::endl
             <<"ticks_in_out: "<<indicator_info.cab.ticks_in_out<<std::endl
             <<"qty_in_out: "<<indicator_info.cab.qty_in_out<<std::endl;
}

void tick(const uint64_t &price, const trade_side_e &side){
    trade_list_t trade_list = set_trade_list(indicator_handler, price, side);
    indicator_handler.process(trade_list);
    print_info("BTCUSDT");
}

int main(void){
    python_script();
    
    tick(80, trade_side_e::buy);
    tick(90, trade_side_e::buy);
    tick(10, trade_side_e::sell);
    return 0;

}
