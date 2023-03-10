#include "manager/indc_config.hpp"

namespace indicators{
    
c_indc_config::c_indc_config(){
    indcs_config_t_ indc_config;
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

    indc_config.config_callback = config::sar::get_config; 
    indc_config.config = &m_sar_config; 
    indc_config.id_number = types_e::SAR;
    indc_config.id_str = "SAR";
    indc_config.source = std::vector<source_e>({source_e::SRC_KLINE});
    indc_config.kline_callback = sar_callback;
    indcs_config_list["sar"] = indc_config;
}

c_indc_config::~c_indc_config(){

}

void c_indc_config::get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
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

void c_indc_config::set(const std::vector<std::vector<std::string>> &indcs,
                        std::array<bool, (uint64_t)types_e::SIZE> &indcs_state,
                        std::array<uint64_t, (uint64_t)source_e::SIZE> &num_of_indcs_per_mngr){
    for(auto &state: indcs_state){
        state = false;
    }
    for(auto &num : num_of_indcs_per_mngr){
        num = 0;
    }

    get_indicators_to_activing_(indcs, indcs_state, num_of_indcs_per_mngr);
}

}
