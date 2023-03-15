#include "manager/indc_mnger.hpp"

#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <array>

/*GLOBAL VAR*/
indicators::indicators_c indicator_handler("indicator");
indicators::indicators_c indicator_symbol_1("SYMBOL_1");
indicators::indicators_c indicator_symbol_2("SYMBOL_2");

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
        {"sar", "a:89.9", "ema:8"},
    };
    indicator_handler.set(indicator_config);
}
/*======================PYTHON SCRIPT======================*/
/*=========================================================*/

indicators::trade_stream_d set_trade_list(double price, indicators::side_e side){

    indicators::trade_stream_d trade_data; 
    trade_data.tokenName = "BTCUSDT";
    trade_data.amountPerPiece = price;
    trade_data.side = side;
    trade_data.eventTime =  std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    return trade_data;
}

void print_info(indicators::indicators_c &indc_handler){
    const indicators::inf_t &indicator_info = indc_handler.get();

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
    indicators::trade_stream_d trade_data_symbol_1 = set_trade_list(price, indicators::side_e::sell);
    indicators::trade_stream_d trade_data_symbol_2 = set_trade_list(price, indicators::side_e::buy);
    indicator_symbol_1.process(trade_data_symbol_1);
    indicator_symbol_2.process(trade_data_symbol_2);
    print_info(indicator_symbol_1);
    print_info(indicator_symbol_2);
}

static void tick_kline_stream(const uint64_t &price){
    indicators::kline_d kline_data;
    indicator_symbol_1.process(kline_data);
    indicator_symbol_2.process(kline_data);
}

int main(void){
    python_script();
    
    tick_trade_stream(80, indicators::side_e::buy);
    
    std::cout<<std::endl;
    tick_kline_stream(90);std::cout<<std::endl;
    return 0;

}
