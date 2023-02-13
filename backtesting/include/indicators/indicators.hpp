#ifndef INDICATORS_HPP_
#define INDICATORS_HPP_
#include "helpers/cllbck_iterator.hpp"
#include "indicators/bwfs/bwfs.hpp"


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
        std::unordered_map<std::string, indicators::indicator_t> m_symbol_list;

        void init_indicators_(void);
        void delete_current_indicators_(void);
        uint64_t calculate_time_threshold_( const uint64_t &timestamp);

        void set_indicators_callbacks_( const std::array<bool, 
                                        (uint64_t)inds_e::SIZE> &indcs);

        void init_BWFS_indicators_(const std::vector<std::string> &itr);

        void get_indicators_to_activing_( const std::vector<std::vector<std::string>> &indcs,
                                          std::array<bool, (uint64_t)inds_e::SIZE> &indcs_state,
                                          std::array<uint64_t, (uint64_t)data_types::SIZE> &num_of_indcs_per_mnger);

        void init_all_indicators_vars_(indicators::indicator_t &indcs);
        auto init_new_symbol_(const std::string symbol); 
    public:
        indicators_c();
        ~indicators_c();

        indicators::indicator_info_t get(const std::string &symbol);

        void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const backtesting::trade_list_t &data);
};

}

#endif