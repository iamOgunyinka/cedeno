#ifndef INDC_CONFIG_HPP_
#define INDC_CONFIG_HPP_
#include <string>
#include <vector>
#include <array>

#include "indc_data.hpp"

namespace indicators{

struct indcs_config_t_{
    using config_callback_t =  void (*)( const std::vector<std::string> &,
                             std::array<bool, (uint64_t)types_e::SIZE>*,
                             std::array<uint64_t, (uint64_t)source_e::SIZE> &,
                             void *,
                             std::vector<source_e> &);
    using trade_callback_t = void (*)( const trade_stream_d &, 
                                       indicator_t &);
    using kline_callback_t = void (*)( const kline_d &, 
                                       indicator_t &);
                                       
    config_callback_t config_callback; 
    trade_callback_t  trade_callback; 
    kline_callback_t  kline_callback; 
    void *config;
    std::string id_str;
    types_e id_number;
    std::vector<source_e> source;
};  

class c_indc_config{
    private:
        void get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                          std::array<bool, (uint64_t)types_e::SIZE> &indcs_state,
                                          std::array<uint64_t, (uint64_t)source_e::SIZE> &num_of_indcs_per_mnger);
    public:
        c_indc_config();
        ~c_indc_config();

        std::unordered_map<std::string, indcs_config_t_> indcs_config_list;

        conf_ema_t m_ema_config;
        conf_BWFS_t m_BWFS_config;
        conf_sma_t m_sma_config;
        conf_macd_t  m_macd_config;
        conf_wma_t  m_wma_config;
        conf_atr_t  m_atr_config;
        conf_sar_t  m_sar_config;

        void set(const std::vector<std::vector<std::string>> &indcs,
                                std::array<bool, (uint64_t)types_e::SIZE> &indcs_state,
                                std::array<uint64_t, (uint64_t)source_e::SIZE> &num_of_indcs_per_mngr);
};


}

#endif