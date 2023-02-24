#include "indicators.hpp"

#include <time.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <memory>

#include "indicators/helpers/indcs_utils.hpp"
#include "indicators/indicators/bwfs/bwfs.hpp"

namespace indicators{

void function( const std::vector<std::string> &,
                             std::array<bool, (uint64_t)types_e::SIZE>*,
                             std::array<uint64_t, (uint64_t)source_e::SIZE> &,
                             void *config){}

indicators_c::indicators_c(){
    m_indcs_trade_mngr = new ind_mngr_c<backtesting::trade_data_t, indicator_t>;

    indcs_config_t indc_config;
    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::TICK_IN;
    indc_config.id_str = "TICK_IN";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = ticks_in_callback;
    indcs_config_list["tick_in"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::TICK_OUT;
    indc_config.id_str = "TICK_OUT";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = ticks_out_callback;
    indcs_config_list["tick_out"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::QTY_IN;
    indc_config.id_str = "QTY_IN";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = qty_in_callback;
    indcs_config_list["qty_in"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::QTY_OUT;
    indc_config.id_str = "QTY_OUT";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = qty_out_callback;
    indcs_config_list["qty_out"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::AVRG_IN;
    indc_config.id_str = "AVRG_IN";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = avrg_in_callback;
    indcs_config_list["avrg_in"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::AVRG_OUT;
    indc_config.id_str = "AVRG_OUT";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = avrg_out_callback;
    indcs_config_list["avrg_out"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::QTY_IN_OUT;
    indc_config.id_str = "QTY_IN_OUT";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = qty_in_out_callback;
    indcs_config_list["qty_in_out"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::TICK_IN_OUT;
    indc_config.id_str = "TICK_IN_OUT";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = tick_in_out_callback;
    indcs_config_list["tick_in_out"] = indc_config;

    indc_config.config_callback = config::bwfs::get_config;
    indc_config.config = &m_BWFS_config;
    indc_config.id_number = types_e::BUY_VS_SELL;
    indc_config.id_str = "BUY_VS_SELL";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.trade_callback = buy_vs_sell_callback;
    indcs_config_list["buy_vs_sell"] = indc_config;

    indc_config.config_callback = config::ema::get_config;
    indc_config.config = &m_ema_config; 
    indc_config.id_number = types_e::EMA;
    indc_config.id_str = "EMA";
    indc_config.source = std::vector<source_e>({source_e::SRC_TRADE});
    indc_config.kline_callback = ema_kline_callback;
    indc_config.trade_callback = ema_trade_callback;
    indcs_config_list["ema"] = indc_config;

    indc_config.config_callback = config::macd::get_config;
    indc_config.config = &m_macd_config;
    indc_config.id_number = types_e::MACD;
    indc_config.id_str = "MACD";
    indc_config.source = std::vector<source_e>({source_e::SRC_KLINE});
    indc_config.kline_callback = macd_callback;
    indcs_config_list["macd"] = indc_config;

    indc_config.config_callback = config::sma::get_config; 
    indc_config.config = &m_sma_config; 
    indc_config.id_number = types_e::SMA;
    indc_config.id_str = "SMA";
    indc_config.source = std::vector<source_e>({source_e::SRC_KLINE});
    indc_config.kline_callback = sma_callback;
    indcs_config_list["sma"] = indc_config;

    indc_config.config_callback = config::wma::get_config; 
    indc_config.config = &m_wma_config; 
    indc_config.id_number = types_e::WMA;
    indc_config.id_str = "WMA";
    indc_config.source = std::vector<source_e>({source_e::SRC_KLINE});
    indc_config.kline_callback = wma_callback;
    indcs_config_list["wma"] = indc_config;

    indc_config.config_callback = config::atr::get_config; 
    indc_config.config = &m_atr_config; 
    indc_config.id_number = types_e::ATR;
    indc_config.id_str = "ATR";
    indc_config.source = std::vector<source_e>({source_e::SRC_KLINE});
    indc_config.kline_callback = atr_callback;
    indcs_config_list["atr"] = indc_config;
}

indicators_c::~indicators_c(){
    delete m_indcs_trade_mngr;
}

void indicators_c::delete_current_indicators_(void){
    delete m_indcs_trade_mngr;
}

void indicators_c::set_indicators_callback(const std::array<bool, (uint64_t)types_e::SIZE> &indcs_state){
    for( uint64_t indc_idx = (uint64_t)types_e::BUY_VS_SELL, end = (uint64_t)types_e::SIZE; 
         indc_idx <  end; 
         indc_idx++){
        if(indcs_state[indc_idx] == true){
            auto indc_config = indcs_config_list.find(indc_list_key_number[indc_idx]);
            for(auto &source : indc_config->second.source){
                if(source == source_e::SRC_TRADE){
                    std::cout<<"Adding " + indc_config->first<<std::endl;
                    m_indcs_trade_mngr->add_indicator(indc_config->second.trade_callback);
                }
                if(source == source_e::SRC_KLINE){
                    std::cout<<"Adding " + indc_config->first<<std::endl;
                    m_indcs_kline_mngr->add_indicator(indc_config->second.kline_callback);
                }
            }
        }
    }
}

void indicators_c::get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                                std::array<bool, (uint64_t)types_e::SIZE> &indcs_state,
                                                std::array<uint64_t, (uint64_t)source_e::SIZE> &num_of_indcs_per_mnger){
    for(auto &itr : indcs){
        auto indc = indcs_config_list.find(itr.front());
        if(indc == indcs_config_list.end())
            std::__throw_runtime_error("Indicator does not exist");

        if(indcs_state[(uint64_t)indc->second.id_number] == true){
            std::__throw_runtime_error(std::string( "You are setting " 
                                                    + indc->second.id_str 
                                                    +" indicator twice").c_str());
        }
        indc->second.config_callback(itr, &indcs_state, num_of_indcs_per_mnger, indc->second.config, indc->second.source);
    }
}

void indicators_c::set(const std::vector<std::vector<std::string>> &indcs){
    delete_current_indicators_();

    std::array<bool, (uint64_t)types_e::SIZE> indcs_state{false};
    std::array<uint64_t, (uint64_t)source_e::SIZE> num_of_indcs_per_mngr{0};    

    get_indicators_to_activing_(indcs, indcs_state, num_of_indcs_per_mngr);
    std::cout<<"Size trade manager: "<<num_of_indcs_per_mngr[(uint64_t)source_e::SRC_TRADE]<<std::endl;
    std::cout<<"Size kline manager: "<<num_of_indcs_per_mngr[(uint64_t)source_e::SRC_KLINE]<<std::endl;
    m_indcs_trade_mngr = new ind_mngr_c<backtesting::trade_data_t,
                                                    indicator_t>(
                                        num_of_indcs_per_mngr[(uint64_t)source_e::SRC_TRADE]
                                        );

    m_indcs_kline_mngr = new ind_mngr_c<kline_test_t, indicator_t>(
                                        num_of_indcs_per_mngr[(uint64_t)source_e::SRC_KLINE]
                                        );
    set_indicators_callback(indcs_state);
}

void indicators_c::init_all_indicators_vars_(indicator_t &indcs){
    indcs.indcs_var.ticks_in_vars = std::make_unique<ticks_in_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.ticks_out_vars = std::make_unique<ticks_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.qtys_in_vars = std::make_unique<qty_in_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.qty_out_vars = std::make_unique<qty_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.avrg_in_vars = std::make_unique<avrg_in_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.avrg_out_vars = std::make_unique<avrg_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.qty_in_out_vars = std::make_unique<qty_in_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.ticks_in_out_vars = std::make_unique<ticks_in_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.buy_vs_sell_vars = std::make_unique<buy_vs_sell_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.ema_vars = std::make_unique<ema_t>(indcs, m_ema_config); 
    indcs.indcs_var.sma_vars = std::make_unique<sma_t>(indcs, m_sma_config); 
    indcs.indcs_var.macd_vars = std::make_unique<macd_t>(indcs, m_macd_config); 
    indcs.indcs_var.wma_vars = std::make_unique<wma_t>(indcs, m_wma_config); 
    indcs.indcs_var.atr_vars = std::make_unique<atr_t>(indcs, m_atr_config); 
}

auto indicators_c::init_new_symbol_(const std::string symbol){
    auto itr = m_symbol_list.emplace(symbol, indicator_t());
    init_all_indicators_vars_(itr.first->second);
    return itr;
}

inf_t indicators_c::get(const std::string &symbol){
    return m_symbol_list[symbol].indc_info;
}

void indicators_c::process(const backtesting::trade_list_t &trade_list){
    const backtesting::trade_data_t &trade_data = trade_list.back(); 
    auto itr = m_symbol_list.find(trade_data.tokenName);
    if(itr == m_symbol_list.end()){
        auto new_symbol = init_new_symbol_(trade_data.tokenName);
        m_indcs_trade_mngr->process(trade_data, new_symbol.first->second);
    }else{
        m_indcs_trade_mngr->process(trade_data, itr->second);
    }   
}

void indicators_c::process(const kline_test_t &kline_data){ 
    auto itr = m_symbol_list.find(kline_data.symbol);
    if(itr == m_symbol_list.end()){
        auto new_symbol = init_new_symbol_(kline_data.symbol);
        m_indcs_kline_mngr->process(kline_data, new_symbol.first->second);
    }else{
        m_indcs_kline_mngr->process(kline_data, itr->second);
    }   
}

}