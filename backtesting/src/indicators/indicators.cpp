#include "indicators.hpp"

#include <time.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <memory>

#include "indicators/bwfs/bwfs_hndlr.hpp"
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
}

void indicators_c::delete_current_indicators_(void){
    delete m_indcs_trade_mngr;
}

void indicators_c::set_indicators_callbacks_(const std::array<bool, (uint64_t)inds_e::SIZE> &indcs){
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
    if(indcs[(uint64_t)inds_e::BWFS_HANDLER] == true){
        m_indcs_trade_mngr->add_indicator(bwfs_hndlr_callback);
    }
}

void indicators_c::get_BWFS_indicator_states_( const std::vector<std::string> &itr, 
                                               std::array<bool, (uint64_t)inds_e::SIZE> &indc_states,
                                               uint64_t &trade_sz, 
                                               uint64_t &config_idx){
    config_idx = 0;
    for(auto &itr_: itr){
        auto item = m_indc_list.find(itr_);
        if( item != m_indc_list.end()){
            indc_states[item->second] = true;
            config_idx++;
            trade_sz++;
        }
    }
    if(config_idx > 0){
        indc_states[(uint64_t)inds_e::BWFS_HANDLER] = true;
        trade_sz++;
    }
}


indicators::ind_BWFS_confg_t indicators_c::get_BWFS_config_( const std::vector<std::string> &config_vector, 
                                                             const uint64_t &indx){
    indicators::ind_BWFS_confg_t config;

    std::for_each(config_vector.begin() + indx, config_vector.end(), [&](const std::string &config_str){
            auto config_pair = indicators::indcs_utils::split_string(config_str, ":"); 
            if(config_pair.first == "mode"){
                if(config_pair.second == "static"){
                    config.mode = ind_mode_e::STATIC;
                }else if(config_pair.second == "dynamic"){
                    config.mode = ind_mode_e::DYNAMIC;
                }else{
                    std::__throw_runtime_error("Wrong BWFS-mode");
                }
            }else if(config_pair.first == "time"){
                config.time = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else if(config_pair.first == "limit"){
                config.limit = strtoul(config_pair.second.c_str(), nullptr, 10);
            }else{
                std::__throw_runtime_error("Wrong BWFS config parameter");
            }
        }
    );
    return config;
}

void indicators_c::check_indc_confg_params_( const std::vector<std::string> &itr, 
                                             const uint64_t &config_idx, 
                                             const uint64_t &max_params_sz, 
                                             const std::string &indc_type){
    int item_left_in_vector = itr.size() - config_idx;
    if(item_left_in_vector > max_params_sz){
        const std::string error_message =   "So many " + 
                                            indc_type + 
                                            " config indicator parameters"; 
        std::__throw_runtime_error(error_message.c_str());
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
                if(indcs_status[(uint64_t)inds_e::BWFS_HANDLER] == true)
                    std::__throw_runtime_error("You are setting BWFS indicator twice");
                uint64_t bwfs_config_idx = 0;
                get_BWFS_indicator_states_( itr, 
                                            indcs_state, 
                                            num_of_indcs_per_mnger[(uint64_t)data_types::TRADE], 
                                            bwfs_config_idx);
                if((bwfs_config_idx + 1) > itr.size()){
                    m_BWFS_config = ind_BWFS_confg_t(); 
                }else{
                    check_indc_confg_params_(itr, bwfs_config_idx, MAX_BWFS_PARAMS_SZ, "BWFS");
                    m_BWFS_config = get_BWFS_config_(itr, bwfs_config_idx);
                }
                std::cout<<"BWFS Config: "<<std::endl<<"time: "<<m_BWFS_config.time<<std::endl; 
                std::cout<<"mode: "<<(int)m_BWFS_config.mode<<std::endl; 
                std::cout<<"limit: "<<m_BWFS_config.limit<<std::endl;
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
    indcs.indcs_var.ticks_in_vars = std::make_unique<indicators::ticks_in_t>(indcs); 
    indcs.indcs_var.ticks_out_vars = std::make_unique<indicators::ticks_out_t>(indcs); 
    indcs.indcs_var.qtys_in_vars = std::make_unique<indicators::qty_in_t>(indcs); 
    indcs.indcs_var.qty_out_vars = std::make_unique<indicators::qty_out_t>(indcs); 
    indcs.indcs_var.avrg_in_vars = std::make_unique<indicators::avrg_in_t>(indcs); 
    indcs.indcs_var.avrg_out_vars = std::make_unique<indicators::avrg_out_t>(indcs); 
    indcs.indcs_var.qty_in_out_vars = std::make_unique<indicators::qty_in_out_t>(indcs); 
    indcs.indcs_var.ticks_in_out_vars = std::make_unique<indicators::ticks_in_out_t>(indcs); 
    indcs.indcs_var.bwfs_hndlr_vars = std::make_unique<indicators::bwfs_hndlr_t>(indcs); 
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