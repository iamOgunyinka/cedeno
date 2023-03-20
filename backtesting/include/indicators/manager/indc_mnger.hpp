#ifndef INDC_MNGER_HPP_
#define INDC_MNGER_HPP_
#include <ctime>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

#include "helpers/cllbck_iterator.hpp"
#include "indc_data.hpp"
#include "manager/indc_config.hpp"



namespace indicators{


class indicators_c{
    private:
        std::string m_id;
        indicator_t m_handler;
        static c_indc_config m_indc_config;

        static ind_mngr_c<trade_stream_d, indicator_t> *m_indcs_trade_mngr;
        static ind_mngr_c<kline_d, indicator_t> *m_indcs_kline_mngr;

        static void delete_current_indicators_(void);

        static void set_indicators_callback_(const std::array<bool, (uint64_t)types_e::SIZE> &indcs);
    public:
        indicators_c(const std::string &id);
        ~indicators_c();

        const inf_t& get(void);

        static void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const trade_stream_d &trade_data);

        void process(const std::vector<kline_d> &kline_data);
};

}

#endif