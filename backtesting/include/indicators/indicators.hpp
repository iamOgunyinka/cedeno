#ifndef INDICATORS_HPP_
#define INDICATORS_HPP_
#include "cllbck_iterator.hpp"

#include <ctime>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

namespace indicators{

enum class data_types{
    TRADE,
    SIZE,
};

class indicators_c{
    private:
        indicators::ind_BWFS_confg_t m_BWFS_config;

        std::unordered_map<std::string, uint64_t> m_indc_list;

        indicators::ind_mngr_c<backtesting::trade_data_t> *m_indcs_trade_mngr;
        std::unordered_map<std::string, indicators::indicator_data_t> m_symbol_list;

        void init_indicators_(void);
        void delete_current_indicators_(void);
        uint64_t calculate_time_threshold_( const uint64_t &timestamp);

        void get_BWFS_indicator_states_( const std::vector<std::string> &itr, 
                                         std::array<bool, (uint64_t)inds_e::SIZE> &indcs,
                                         uint64_t &trade_sz, 
                                         uint64_t &config_idx);

        void check_indc_confg_params_( const std::vector<std::string> &itr, 
                                       const uint64_t &config_idx, 
                                       const uint64_t &max_params_sz, 
                                       const std::string &indc_type);

        void set_indicators_callbacks_( const std::array<bool, 
                                        (uint64_t)inds_e::SIZE> &indcs);

        void init_BWFS_indicators_(const std::vector<std::string> &itr);

        indicators::ind_BWFS_confg_t get_BWFS_config_( const std::vector<std::string> &itr, 
                                                       const uint64_t &indx);

        void get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                          std::array<bool, (uint64_t)inds_e::SIZE> &indcs_state,
                                          std::array<uint64_t, (uint64_t)data_types::SIZE> &num_of_indcs_per_mnger);

        void init_all_indicators_vars_(indicators::indicator_data_t &indcs);
        auto init_new_symbol_(const std::string symbol); 
    public:
        indicators_c();
        ~indicators_c();

        indicators::indicator_data_t get_indicator_data(const std::string &symbol);

        void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const backtesting::trade_list_t &data);
};

}

#endif