#ifndef INDC_MNGER_HPP_
#define INDC_MNGER_HPP_
#include <ctime>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

#include "helpers/cllbck_iterator.hpp"
#include "indc_data.hpp"
#include "source_data.hpp"
#include "manager/indc_config.hpp"



namespace indicators{


class indicators_c{
    private:
        std::string m_id;
        indicator_t m_handler;
        c_indc_config m_indc_config;

        ind_mngr_c<trade_stream_d, indicator_t> *m_indcs_trade_mngr;
        ind_mngr_c<kline_d, indicator_t> *m_indcs_kline_mngr;

        void delete_current_indicators_(void);

        void init_all_indicators_vars_(indicator_t &indcs);
        auto init_new_symbol_(const std::string symbol); 
        void set_indicators_callback(const std::array<bool, (uint64_t)types_e::SIZE> &indcs);
    public:
        indicators_c(const std::string &id);
        ~indicators_c();

        const inf_t& get(void);

        void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const trade_stream_d &trade_data);

        void process(const kline_d &kline_data);
};

}

#endif