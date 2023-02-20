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
    indcs_config_t( config_callback_t config_callback_, 
                    void *config_,
                    const indicators::types_e &id_number_,
                    const std::string &id_str_):
                    config_callback(config_callback_),
                    config(config_),
                    id_number(id_number_),
                    id_str(id_str_)
                    {}

    config_callback_t config_callback; 
    void *config;
    std::string id_str;
    indicators::types_e id_number;
    std::array<std::string, (uint64_t)data_types::SIZE> source_data;
};  

class indicators_c{
    private:
        std::unordered_map<std::string, indcs_config_t> indcs_config;

        indicators::conf_ema_t m_ema_config;
        indicators::conf_BWFS_t m_BWFS_config;
        indicators::conf_sma_t m_sma_config;
        indicators::conf_macd_t  m_macd_config;

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