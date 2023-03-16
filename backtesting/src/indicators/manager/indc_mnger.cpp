#include "manager/indc_mnger.hpp"

#include <time.h>
#include <bits/stdc++.h>
#include <stdio.h>
#include <memory>

#include "indicators/helpers/indcs_utils.hpp"
#include "indicators/indicators/bwfs/bwfs.hpp"

namespace indicators{

c_indc_config indicators_c::m_indc_config;
ind_mngr_c<trade_stream_d, indicator_t> *indicators_c::m_indcs_trade_mngr;
ind_mngr_c<kline_d, indicator_t> *indicators_c::m_indcs_kline_mngr;

indicators_c::indicators_c(const std::string &id){
    m_id = id;

    m_indcs_trade_mngr = new ind_mngr_c<trade_stream_d, indicator_t>;

    m_handler.indcs_var.ticks_in_vars = std::make_unique<ticks_in_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.ticks_out_vars = std::make_unique<ticks_out_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.qtys_in_vars = std::make_unique<qty_in_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.qty_out_vars = std::make_unique<qty_out_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.avrg_in_vars = std::make_unique<avrg_in_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.avrg_out_vars = std::make_unique<avrg_out_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.qty_in_out_vars = std::make_unique<qty_in_out_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.ticks_in_out_vars = std::make_unique<ticks_in_out_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.buy_vs_sell_vars = std::make_unique<buy_vs_sell_t>(m_handler, indicators_c::m_indc_config.m_BWFS_config); 
    m_handler.indcs_var.ema_vars = std::make_unique<ema_t>(m_handler, indicators_c::m_indc_config.m_ema_config); 
    m_handler.indcs_var.sma_vars = std::make_unique<sma_t>(m_handler, indicators_c::m_indc_config.m_sma_config); 
    m_handler.indcs_var.macd_vars = std::make_unique<macd_t>(m_handler, indicators_c::m_indc_config.m_macd_config); 
    m_handler.indcs_var.wma_vars = std::make_unique<wma_t>(m_handler, indicators_c::m_indc_config.m_wma_config); 
    m_handler.indcs_var.atr_vars = std::make_unique<atr_t>(m_handler, indicators_c::m_indc_config.m_atr_config); 
    m_handler.indcs_var.sar_vars = std::make_unique<sar_t>(m_handler, indicators_c::m_indc_config.m_sar_config); 
}

indicators_c::~indicators_c(){
    delete indicators_c::m_indcs_trade_mngr;
    delete indicators_c::m_indcs_kline_mngr;
}

void indicators_c::delete_current_indicators_(void){
    delete indicators_c::m_indcs_trade_mngr;
    delete indicators_c::m_indcs_kline_mngr;
}

void indicators_c::set_indicators_callback_(const std::array<bool, (uint64_t)types_e::SIZE> &indcs_state){
    for( uint64_t indc_idx = (uint64_t)types_e::BUY_VS_SELL, end = (uint64_t)types_e::SIZE; 
         indc_idx <  end; 
         indc_idx++){
        if(indcs_state[indc_idx] == true){
            auto indc_config = indicators_c::m_indc_config.indcs_config_list.find(indc_list_key_number[indc_idx]);
            for(auto &source : indc_config->second.source){
                if(source == source_e::SRC_TRADE){
                    indicators_c::m_indcs_trade_mngr->add_indicator(indc_config->second.trade_callback);
                }
                if(source == source_e::SRC_KLINE){
                    indicators_c::m_indcs_kline_mngr->add_indicator(indc_config->second.kline_callback);
                }
            }
        }
    }
}

void indicators_c::set(const std::vector<std::vector<std::string>> &indcs){
    delete_current_indicators_();

    std::array<bool, (uint64_t)types_e::SIZE> indcs_state{false};
    std::array<uint64_t, (uint64_t)source_e::SIZE> num_of_indcs_per_mngr{0};    

    indicators_c::m_indc_config.set(indcs, indcs_state, num_of_indcs_per_mngr);

    indicators_c::m_indcs_trade_mngr = new ind_mngr_c<trade_stream_d,indicator_t>(
                                        num_of_indcs_per_mngr[(uint64_t)source_e::SRC_TRADE]
                                        );

    indicators_c::m_indcs_kline_mngr = new ind_mngr_c<kline_d, indicator_t>(
                                        num_of_indcs_per_mngr[(uint64_t)source_e::SRC_KLINE]
                                        );
    set_indicators_callback_(indcs_state);
}

const inf_t& indicators_c::get(void){
    return m_handler.info;
}

void indicators_c::process(const trade_stream_d &trade_data){
    m_indcs_trade_mngr->process(trade_data, m_handler);
}

void indicators_c::process(const std::vector<kline_d> &kline_data_list){
  for (auto const &kline_data: kline_data_list) {
    m_indcs_kline_mngr->process(kline_data, m_handler);
  }
}

}