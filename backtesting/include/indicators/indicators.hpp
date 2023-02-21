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
                             std::array<uint64_t, (uint64_t)data_types::SIZE> &,
                             void *config);
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
    indicators::types_e id_number;
    std::vector<indicators::data_types> source;
};  

class indicators_c{
    private:
        std::unordered_map<std::string, indcs_config_t> indcs_config_list;

        indicators::conf_ema_t m_ema_config;
        indicators::conf_BWFS_t m_BWFS_config;
        indicators::conf_sma_t m_sma_config;
        indicators::conf_macd_t  m_macd_config;
        indicators::conf_wma_t  m_wma_config;

        indicators::ind_mngr_c<backtesting::trade_data_t, indicators::indicator_t> *m_indcs_trade_mngr;
        indicators::ind_mngr_c<kline_test_t, indicators::indicator_t> *m_indcs_kline_mngr;
        std::unordered_map<std::string, indicators::indicator_t> m_symbol_list;

        void delete_current_indicators_(void);
        uint64_t calculate_time_threshold_( const uint64_t &timestamp);

        void set_trade_stream_indicators( const std::array<bool, 
                                        (uint64_t)types_e::SIZE> &indcs);

        void set_kline_stream_indicators( const std::array<bool, 
                                        (uint64_t)types_e::SIZE> &indcs);

        void init_BWFS_indicators_(const std::vector<std::string> &itr);

        void get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                          std::array<bool, (uint64_t)types_e::SIZE> &indcs_state,
                                          std::array<uint64_t, (uint64_t)data_types::SIZE> &num_of_indcs_per_mnger);

        void init_all_indicators_vars_(indicators::indicator_t &indcs);
        auto init_new_symbol_(const std::string symbol); 
        void set_indicators_callback(const std::array<bool, (uint64_t)types_e::SIZE> &indcs);
    public:
        indicators_c();
        ~indicators_c();

        indicators::inf_t get(const std::string &symbol);

        void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const backtesting::trade_list_t &data);

        void process(const kline_test_t &kline_data);
};

}

#endif