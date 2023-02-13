#include "indicators.hpp"

#include <time.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <memory>

#include "indicators/helpers/indcs_utils.hpp"

namespace indicators{

#define MAX_BWFS_PARAMS_SZ 3

indicators_c::indicators_c(){
    m_indcs_trade_mngr = new indicators::ind_mngr_c<backtesting::trade_data_t>;
    init_indicators_();
}

indicators_c::~indicators_c(){
    delete m_indcs_trade_mngr;
}

void indicators_c::init_indicators_(void){
    m_indc_list["tick_in"]      = (uint64_t)inds_e::TICK_IN;
    m_indc_list["tick_out"]     = (uint64_t)inds_e::TICK_OUT;
    m_indc_list["qty_in"]       = (uint64_t)inds_e::QTY_IN;
    m_indc_list["qty_out"]      = (uint64_t)inds_e::QTY_OUT;
    m_indc_list["avrg_in"]      = (uint64_t)inds_e::AVRG_IN;
    m_indc_list["avrg_out"]     = (uint64_t)inds_e::AVRG_OUT;
    m_indc_list["qty_in_out"]   = (uint64_t)inds_e::QTY_IN_OUT;
    m_indc_list["tick_in_out"]  = (uint64_t)inds_e::TICK_IN_OUT;
    m_indc_list["buy_vs_sell"]  = (uint64_t)inds_e::BUY_VS_SELL;
}

void indicators_c::delete_current_indicators_(void){
    delete m_indcs_trade_mngr;
}

void indicators_c::set_indicators_callbacks_(const std::array<bool, (uint64_t)inds_e::SIZE> &indcs){
    if(indcs[(uint64_t)inds_e::BUY_VS_SELL] == true){
        m_indcs_trade_mngr->add_indicator(buy_vs_sell_callback);
    }
    if(indcs[(uint64_t)inds_e::TICK_IN] == true){
        m_indcs_trade_mngr->add_indicator(ticks_in_callback);
    }
    if(indcs[(uint64_t)inds_e::TICK_OUT] == true){
        m_indcs_trade_mngr->add_indicator(ticks_out_callback);
    }
    if(indcs[(uint64_t)inds_e::QTY_IN] == true){
        m_indcs_trade_mngr->add_indicator(qty_in_callback);
    }
    if(indcs[(uint64_t)inds_e::QTY_OUT] == true){
        m_indcs_trade_mngr->add_indicator(qty_out_callback);
    }
    if(indcs[(uint64_t)inds_e::AVRG_IN] == true){
        m_indcs_trade_mngr->add_indicator(avrg_in_callback);
    }
    if(indcs[(uint64_t)inds_e::AVRG_OUT] == true){
        m_indcs_trade_mngr->add_indicator(avrg_out_callback);
    }
    if(indcs[(uint64_t)inds_e::QTY_IN_OUT] == true){
        m_indcs_trade_mngr->add_indicator(qty_in_out_callback);
    }
    if(indcs[(uint64_t)inds_e::TICK_IN_OUT] == true){
        m_indcs_trade_mngr->add_indicator(ticks_in_out_callback);
    }
}

void indicators_c::get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                                std::array<bool, (uint64_t)inds_e::SIZE> &indcs_state,
                                                std::array<uint64_t, (uint64_t)data_types::SIZE> &num_of_indcs_per_mnger){
    std::array<bool, (uint64_t)inds_e::SIZE> indcs_status{false};    
    for(auto &itr : indcs){
        uint64_t indc_idx =  m_indc_list.find(itr.front())->second; 
        switch (indc_idx){
            case (uint64_t)inds_e::TICK_IN:
            case (uint64_t)inds_e::TICK_OUT:
            case (uint64_t)inds_e::QTY_IN:
            case (uint64_t)inds_e::QTY_OUT:
            case (uint64_t)inds_e::AVRG_IN:
            case (uint64_t)inds_e::AVRG_OUT:
            case (uint64_t)inds_e::QTY_IN_OUT:
            case (uint64_t)inds_e::TICK_IN_OUT:
            {
                if(indcs_status[(uint64_t)inds_e::BWFS_HANDLER] == true){
                    std::__throw_runtime_error("You are setting BWFS indicator twice");
                }
                m_BWFS_config = indicators::bwfs::get_config(itr, indcs_state, num_of_indcs_per_mnger[(uint64_t)data_types::TRADE]);
                indcs_status[(uint64_t)inds_e::BWFS_HANDLER] = true;
                break;
            }
            default:
                std::__throw_runtime_error("Indicator does not exist");
                break;
        }
    }
}

void indicators_c::set(const std::vector<std::vector<std::string>> &indcs){
    delete_current_indicators_();

    std::array<bool, (uint64_t)inds_e::SIZE> indcs_state{false};
    std::array<uint64_t, (uint64_t)data_types::SIZE> num_of_indcs_per_mngr{0};    

    get_indicators_to_activing_(indcs, indcs_state, num_of_indcs_per_mngr);

    m_indcs_trade_mngr = new indicators::ind_mngr_c<backtesting::trade_data_t>(
                                        num_of_indcs_per_mngr[(uint64_t)data_types::TRADE]
                                        );
    set_indicators_callbacks_(indcs_state);
}

void indicators_c::init_all_indicators_vars_(indicators::indicator_t &indcs){
    indcs.indcs_var.ticks_in_vars = std::make_unique<indicators::ticks_in_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.ticks_out_vars = std::make_unique<indicators::ticks_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.qtys_in_vars = std::make_unique<indicators::qty_in_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.qty_out_vars = std::make_unique<indicators::qty_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.avrg_in_vars = std::make_unique<indicators::avrg_in_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.avrg_out_vars = std::make_unique<indicators::avrg_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.qty_in_out_vars = std::make_unique<indicators::qty_in_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.ticks_in_out_vars = std::make_unique<indicators::ticks_in_out_t>(indcs, m_BWFS_config); 
    indcs.indcs_var.buy_vs_sell_vars = std::make_unique<indicators::buy_vs_sell_t>(indcs, m_BWFS_config); 
}

auto indicators_c::init_new_symbol_(const std::string symbol){
    auto itr = m_symbol_list.emplace(symbol, indicators::indicator_t());
    init_all_indicators_vars_(itr.first->second);
    return itr;
}

indicators::indicator_info_t indicators_c::get(const std::string &symbol){
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

}