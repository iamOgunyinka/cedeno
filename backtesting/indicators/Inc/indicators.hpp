#ifndef INDICATORS_HPP_
#define INDICATORS_HPP_
#include "indicator_manager.hpp"
#include <ctime>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

#include "BWFS/ticks_in.hpp"
#include "BWFS/ticks_out.hpp"
#include "BWFS/qty_in.hpp"
#include "BWFS/qty_out.hpp"
#include "BWFS/avrg_in.hpp"
#include "BWFS/avrg_out.hpp"
#include "BWFS/qty_in_out.hpp"
#include "BWFS/ticks_in_out.hpp"

namespace indicators{

class indicators_c{
    private:
        indicators::ind_BWFS_confg_t BWFS_config;
        ticks_in_t      m_ticks_in; 
        ticks_out_t     m_ticks_out; 
        qty_in_t        m_qty_in; 
        qty_out_t       m_qty_out; 
        avrg_in_t       m_avrg_in; 
        avrg_out_t      m_avrg_out; 
        qty_in_out_t    m_qty_in_out; 
        ticks_in_out_t  m_ticks_in_out; 

        uint64_t *m_ind_confg_sz[(uint64_t)inds_e::SIZE];
        void *m_ind_confg[(uint64_t)inds_e::SIZE];
        indicators::indicator_data_t indicator_data;
        struct{
            bool set_threshold;
            uint64_t time_threshold;
            indicators::ind_BWFS_confg_t *confg;
        }m_BWFS_vars;

        struct indcs_setting_t{
            indcs_setting_t(){

            }
            void *indicator_handler;
            std::string indicator_group;
        };
        

        std::unordered_map<std::string, uint64_t> indc_list;

        
        indicators::ind_mngr_c<backtesting::trade_data_t> *m_bwfs_ind_hndlr;
        void init_indicators_(void);
        void del_creat_indicators_(void);
        uint64_t calculate_time_threshold_( const uint64_t &timestamp);

        void get_BWFS_indicators_state_(  const std::vector<std::string> &itr, 
                                            std::array<bool, (uint64_t)inds_e::SIZE> &indcs,
                                            uint64_t &trade_sz, 
                                            uint64_t &config_idx);
        void check_correct_members_config_size_(const std::vector<std::string> &itr, const uint64_t &config_idx, const uint64_t &max_params_sz, const std::string &indc_type);
        void set_indicators_callbacks_(const std::array<bool, (uint64_t)inds_e::SIZE> &indcs);
        void init_BWFS_indicators_(const std::vector<std::string> &itr);
        indicators::ind_BWFS_confg_t get_BWFS_config_(const std::vector<std::string> &itr, const uint64_t &indx);
    public:
        indicators_c();
        ~indicators_c();

        void enable_indicator(const indicators::inds_e &ind);
        void disable_indicator(const indicators::inds_e &ind);

        void set(const std::vector<std::vector<std::string>> &indcs);

        void process(const backtesting::trade_list_t &data);
};

}

#endif