#include "indicators.hpp"

#include <time.h>
#include <bits/stdc++.h>
#include <stdio.h>

namespace indicators{

#define MAX_BWFS_PARAMS_SZ 3

indicators_c::indicators_c():
                                m_ticks_in(indicator_data),
                                m_ticks_out(indicator_data),
                                m_qty_in(indicator_data),
                                m_qty_out(indicator_data),
                                m_avrg_in(indicator_data),
                                m_avrg_out(indicator_data),
                                m_qty_in_out(indicator_data),
                                m_ticks_in_out(indicator_data)
                            {
    m_BWFS_vars = {
        .set_threshold = true,
        .time_threshold = 0
    };
    m_bwfs_ind_hndlr = new indicators::ind_mngr_c<backtesting::trade_data_t>;
    init_indicators_();
}

indicators_c::~indicators_c(){
    delete m_bwfs_ind_hndlr;
}

void indicators_c::init_indicators_(void){
    indc_list["tick_in"]      = (uint64_t)inds_e::TICK_IN;
    indc_list["tick_out"]     = (uint64_t)inds_e::TICK_OUT;
    indc_list["qty_in"]       = (uint64_t)inds_e::QTY_IN;
    indc_list["qty_out"]      = (uint64_t)inds_e::QTY_OUT;
    indc_list["avrg_in"]      = (uint64_t)inds_e::AVRG_IN;
    indc_list["avrg_out"]     = (uint64_t)inds_e::AVRG_OUT;
    indc_list["qty_in_out"]   = (uint64_t)inds_e::QTY_IN_OUT;
    indc_list["tick_in_out"]  = (uint64_t)inds_e::TICK_IN_OUT;
}

void indicators_c::del_creat_indicators_(void){
    delete m_bwfs_ind_hndlr;
}

void indicators_c::set_indicators_callbacks_(const std::array<bool, (uint64_t)inds_e::SIZE> &indcs){
    if(indcs[(uint64_t)bwfs_inds_e::TICK_IN] == true){
        m_ticks_in.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(ticks_in_callback,(intptr_t)&m_ticks_in);
    }
    if(indcs[(uint64_t)bwfs_inds_e::TICK_OUT] == true){
        m_ticks_out.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(ticks_out_callback,(intptr_t)&m_ticks_out);
    }
    if(indcs[(uint64_t)bwfs_inds_e::QTY_IN] == true){
        m_qty_in.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(qty_in_callback,(intptr_t)&m_qty_in);
    }
    if(indcs[(uint64_t)bwfs_inds_e::QTY_OUT] == true){
        m_qty_out.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(qty_out_callback,(intptr_t)&m_qty_out);
    }
    if(indcs[(uint64_t)bwfs_inds_e::AVRG_IN] == true){
        m_avrg_in.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(avrg_in_callback,(intptr_t)&m_avrg_in);
    }
    if(indcs[(uint64_t)bwfs_inds_e::AVRG_OUT] == true){
        m_avrg_out.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(avrg_out_callback,(intptr_t)&m_avrg_out);
    }
    if(indcs[(uint64_t)bwfs_inds_e::QTY_IN_OUT] == true){
        m_qty_in_out.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(qty_in_out_callback,(intptr_t)&m_qty_in_out);
    }
    if(indcs[(uint64_t)bwfs_inds_e::TICK_IN_OUT] == true){
        m_ticks_in_out.config(BWFS_config);
        m_bwfs_ind_hndlr->add_indicator(ticks_in_out_callback,(intptr_t)&m_ticks_in_out);
    }
}

void indicators_c::get_BWFS_indicators_state_( const std::vector<std::string> &itr, 
                                               std::array<bool, (uint64_t)inds_e::SIZE> &indcs,
                                               uint64_t &trade_sz, 
                                               uint64_t &config_idx){
    config_idx = 0;
    for(auto &itr_: itr){
        auto item = indc_list.find(itr_);
        if( item != indc_list.end()){
            indcs[item->second] = true;
            config_idx++;
            trade_sz++;
        }
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

void indicators_c::check_correct_members_config_size_( const std::vector<std::string> &itr, 
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

void indicators_c::set(const std::vector<std::vector<std::string>> &indcs){
    del_creat_indicators_();

    uint64_t trade_sz = 0;
    std::array<bool, (uint64_t)inds_e::SIZE> indcs_state{false};
    
    bool BWFS_status = false;

    for(auto &itr : indcs){
        uint64_t indc_idx =  indc_list.find(itr.front())->second; 
        switch (indc_idx){
            case (uint64_t)inds_e::TICK_IN:
            case (uint64_t)inds_e::TICK_OUT:
            case (uint64_t)inds_e::QTY_IN:
            case (uint64_t)inds_e::QTY_OUT:
            case (uint64_t)inds_e::AVRG_IN:
            case (uint64_t)inds_e::AVRG_OUT:
            case (uint64_t)inds_e::QTY_IN_OUT:
            case (uint64_t)inds_e::TICK_IN_OUT:
                if(BWFS_status == true)
                    std::__throw_runtime_error("You are setting BWFS indicator twice");
                uint64_t bwfs_config_idx = 0;
                get_BWFS_indicators_state_(itr, indcs_state, trade_sz, bwfs_config_idx);
                check_correct_members_config_size_(itr, bwfs_config_idx, MAX_BWFS_PARAMS_SZ, "BWFS");
                BWFS_config = get_BWFS_config_(itr, bwfs_config_idx);
                BWFS_status = true;
                break;
            default:
                break;
        }
    }
    m_bwfs_ind_hndlr = new indicators::ind_mngr_c<backtesting::trade_data_t>(trade_sz);
    std::cout<<"trade size: "<<trade_sz<<std::endl;
    set_indicators_callbacks_(indcs_state); 
}

void indicators_c::disable_indicator(const indicators::inds_e &ind){
}

void indicators_c::enable_indicator(const indicators::inds_e &ind){
}

uint64_t indicators_c::calculate_time_threshold_(const uint64_t &timestamp){
    time_t time_in_sec = timestamp/1000;
    if(m_BWFS_vars.confg->mode == indicators::ind_mode_e::STATIC){
        return (time_in_sec + m_BWFS_vars.confg->time*60)*1000;
    }else{
        struct tm time = *localtime(&time_in_sec);
        uint64_t counter = 0;
        while(counter < time.tm_min){
            counter += m_BWFS_vars.confg->time;
        }
        uint64_t dif_secs = (counter - time.tm_min)*60;
        return (time_in_sec + dif_secs)*1000;
    }
}

void indicators_c::process(const backtesting::trade_list_t &trade_list){
    const backtesting::trade_data_t &trade_data = trade_list.back(); 
    m_bwfs_ind_hndlr->process(trade_data);
    // ind_db_itr_t itr = m_bwfs_ind_hndlr->process(trade_data, new_data);
    // if(m_BWFS_vars.set_threshold == true){
    //     m_BWFS_vars.time_threshold = calculate_time_threshold_(trade_data.eventTime);
    //     m_BWFS_vars.set_threshold = false;
    // }else{
    //     if(trade_data.eventTime > m_BWFS_vars.time_threshold){
    //         if(m_BWFS_vars.confg->mode == indicators::ind_mode_e::STATIC){
    //             {
    //                 indicators::indicators_list_t empty_list;
    //                 std::swap(itr->second, empty_list);
    //             }
    //             itr->second.push(new_data);
    //             m_BWFS_vars.time_threshold += m_BWFS_vars.confg->time*60*1000;
    //         }else{
    //             itr->second.pop();
    //             itr->second.push(new_data);
    //         }
    //     }
    // }
}


}