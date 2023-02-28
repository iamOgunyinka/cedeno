#include "indicators.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <array>

/*GLOBAL VAR*/
indicators::indicators_c indicator_handler;

/*=========================================================*/
/*======================PYTHON SCRIPT======================*/

void python_script(void){
    std::vector<std::vector<std::string>> indicator_config{
        {"qty_out", "qty_in", "tick_in","tick_out", "avrg_out", "avrg_in","buy_vs_sell", "mode:dynamic", "client_confirmation:878","time:45"}, /*config mode and time*/
        {"ema", "n:8"},
        {"sma", "n:80"},
        {"macd"},
        {"wma", "n:5", "w:1,2.9,3.91,6,3.5"},
        {"atr", "n:10"},
    };
    indicator_handler.set(indicator_config);
}
/*======================PYTHON SCRIPT======================*/
/*=========================================================*/



indicators::trade_stream_d set_trade_list(double price, indicators::side_e side){

    indicators::trade_stream_d trade_list;
    indicators::trade_stream_d trade_data; 
    trade_data.tokenName = "BTCUSDT";
    trade_data.amountPerPiece = price;
    trade_data.side = side;
    trade_data.eventTime =  std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return trade_list;
}

void print_info(const std::string &symbol){
    const indicators::inf_t &indicator_info = indicator_handler.get();

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

static void tick_trade_stream(const uint64_t &price, const indicators::side_e &side){
    indicators::trade_stream_d trade_list = set_trade_list(price, side);
    indicator_handler.process(trade_list);
    print_info("BTCUSDT");
}

static void tick_kline_stream(const uint64_t &price){
    indicators::kline_d kline_data;
    indicator_handler.process(kline_data);
}

int main(void){
    python_script();
    
    tick_trade_stream(80, indicators::side_e::buy);
    tick_trade_stream(90, indicators::side_e::buy);
    tick_trade_stream(10, indicators::side_e::sell);
    
    tick_kline_stream(90);
    tick_kline_stream(80);
    tick_kline_stream(80);
    tick_kline_stream(80);
    tick_kline_stream(80);
    return 0;

}
