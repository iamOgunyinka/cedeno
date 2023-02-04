#include "indicators.hpp"

#include <time.h>
#include <bits/stdc++.h>
#include <stdio.h>

namespace indicators{

#define MAX_BWFS_PARAMS_SZ 3

indicators_c::indicators_c():
                                m_ticks_in(m_global_indicator_data),
                                m_ticks_out(m_global_indicator_data),
                                m_qty_in(m_global_indicator_data),
                                m_qty_out(m_global_indicator_data),
                                m_avrg_in(m_global_indicator_data),
                                m_avrg_out(m_global_indicator_data),
                                m_qty_in_out(m_global_indicator_data),
                                m_ticks_in_out(m_global_indicator_data),
                                m_bwfs_hndlr(m_global_indicator_data)
                            {
    m_BWFS_vars = {
        .set_threshold = true,
        .time_threshold = 0
    };
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
        m_ticks_in.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(ticks_in_callback,(intptr_t)&m_ticks_in);
    }
    if(indcs[(uint64_t)inds_e::TICK_OUT] == true){
        m_ticks_out.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(ticks_out_callback,(intptr_t)&m_ticks_out);
    }
    if(indcs[(uint64_t)inds_e::QTY_IN] == true){
        m_qty_in.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(qty_in_callback,(intptr_t)&m_qty_in);
    }
    if(indcs[(uint64_t)inds_e::QTY_OUT] == true){
        m_qty_out.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(qty_out_callback,(intptr_t)&m_qty_out);
    }
    if(indcs[(uint64_t)inds_e::AVRG_IN] == true){
        m_avrg_in.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(avrg_in_callback,(intptr_t)&m_avrg_in);
    }
    if(indcs[(uint64_t)inds_e::AVRG_OUT] == true){
        m_avrg_out.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(avrg_out_callback,(intptr_t)&m_avrg_out);
    }
    if(indcs[(uint64_t)inds_e::QTY_IN_OUT] == true){
        m_qty_in_out.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(qty_in_out_callback,(intptr_t)&m_qty_in_out);
    }
    if(indcs[(uint64_t)inds_e::TICK_IN_OUT] == true){
        m_ticks_in_out.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(ticks_in_out_callback,(intptr_t)&m_ticks_in_out);
    }
    if(indcs[(uint64_t)inds_e::BWFS_HANDLER] == true){
        m_bwfs_hndlr.config(m_BWFS_config);
        m_indcs_trade_mngr->add_indicator(bwfs_hndlr_callback,(intptr_t)&m_bwfs_hndlr);
    }
}

void indicators_c::get_BWFS_indicators_state_( const std::vector<std::string> &itr, 
                                               std::array<bool, (uint64_t)inds_e::SIZE> &indcs,
                                               uint64_t &trade_sz, 
                                               uint64_t &config_idx){
    config_idx = 0;
    for(auto &itr_: itr){
        auto item = m_indc_list.find(itr_);
        if( item != m_indc_list.end()){
            indcs[item->second] = true;
            config_idx++;
            trade_sz++;
        }
    }
    if(config_idx > 0){
        indcs[(uint64_t)inds_e::BWFS_HANDLER] = true;
        trade_sz++;
    }
}

indicators::ind_BWFS_confg_t indicators_c::get_BWFS_config_( const std::vector<std::string> &itr, 
                                                             const uint64_t &indx){
    indicators::ind_BWFS_confg_t config;

    if(itr[indx] == "static"){
        config.mode = ind_mode_e::STATIC;
    }else if(itr[indx] == "dynamic"){
        config.mode = ind_mode_e::DYNAMIC;
    }else{
        std::__throw_runtime_error("Wrong BWFS mode");
    }
    config.time = strtoul(itr[indx + 1].c_str(), nullptr, 10);
    config.limit = strtoul(itr[indx + 2].c_str(), nullptr, 10);
    return config;
}

void indicators_c::check_indc_confg_params_( const std::vector<std::string> &itr, 
                                             const uint64_t &config_idx, 
                                             const uint64_t &max_params_sz, 
                                             const std::string &indc_type){
    int item_left_in_vector = itr.size() - config_idx;
    if(item_left_in_vector != max_params_sz){
        const std::string error_message =   "Wrong number of " + 
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
                get_BWFS_indicators_state_( itr, 
                                            indcs_state, 
                                            num_of_indcs_per_mnger[(uint64_t)data_types::TRADE], 
                                            bwfs_config_idx);
                check_indc_confg_params_(itr, bwfs_config_idx, MAX_BWFS_PARAMS_SZ, "BWFS");
                m_BWFS_config = get_BWFS_config_(itr, bwfs_config_idx);
                indcs_status[(uint64_t)inds_e::BWFS_HANDLER] = true;
                break;
            }
            default:
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
    std::cout<<"trade size: "<<num_of_indcs_per_mngr[(uint64_t)data_types::TRADE]<<std::endl;
    set_indicators_callbacks_(indcs_state);
}

void indicators_c::process(const backtesting::trade_list_t &trade_list){
    const backtesting::trade_data_t &trade_data = trade_list.back(); 
    m_indcs_trade_mngr->process(trade_data);

}


}