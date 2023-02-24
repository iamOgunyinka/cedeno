#ifndef INDICATORS_HPP_
#define INDICATORS_HPP_
#include "helpers/cllbck_iterator.hpp"
#include "indc_data.hpp"
#include "testing.hpp"


#include <ctime>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

namespace indicators{

struct indcs_config_t{
    using config_callback_t =  void (*)( const std::vector<std::string> &,
                             std::array<bool, (uint64_t)types_e::SIZE>*,
                             std::array<uint64_t, (uint64_t)source_e::SIZE> &,
                             void *,
                             std::vector<source_e> &);
    using trade_callback_t = void (*)( const backtesting::trade_data_t &, 
                                       indicator_t &);
    using kline_callback_t = void (*)( const kline_test_t &, 
                                       indicator_t &);

    indcs_config_t(){}

    config_callback_t config_callback; 
    trade_callback_t  trade_callback; 
    kline_callback_t  kline_callback; 
    void *config;
    std::string id_str;
    types_e id_number;
    std::vector<source_e> source;
};  

class indicators_c{
    private:
        std::unordered_map<std::string, indcs_config_t> indcs_config_list;

        conf_ema_t m_ema_config;
        conf_BWFS_t m_BWFS_config;
        conf_sma_t m_sma_config;
        conf_macd_t  m_macd_config;
        conf_wma_t  m_wma_config;
        conf_atr_t  m_atr_config;

        ind_mngr_c<backtesting::trade_data_t, indicator_t> *m_indcs_trade_mngr;
        ind_mngr_c<kline_test_t, indicator_t> *m_indcs_kline_mngr;
        std::unordered_map<std::string, indicator_t> m_symbol_list;

        void delete_current_indicators_(void);
        uint64_t calculate_time_threshold_( const uint64_t &timestamp);

        void set_trade_stream_indicators( const std::array<bool, 
                                        (uint64_t)types_e::SIZE> &indcs);

        void set_kline_stream_indicators( const std::array<bool, 
                                        (uint64_t)types_e::SIZE> &indcs);

        void init_BWFS_indicators_(const std::vector<std::string> &itr);

        void get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                          std::array<bool, (uint64_t)types_e::SIZE> &indcs_state,
                                          std::array<uint64_t, (uint64_t)source_e::SIZE> &num_of_indcs_per_mnger);

        void init_all_indicators_vars_(indicator_t &indcs);
        auto init_new_symbol_(const std::string symbol); 
        void set_indicators_callback(const std::array<bool, (uint64_t)types_e::SIZE> &indcs);
    public:
        indicators_c();
        ~indicators_c();

        inf_t get(const std::string &symbol);

        void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const backtesting::trade_list_t &data);

        void process(const kline_test_t &kline_data);
};

}

#endif