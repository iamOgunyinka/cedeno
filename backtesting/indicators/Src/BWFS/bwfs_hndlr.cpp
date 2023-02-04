#include "BWFS/bwfs_hndlr.hpp"

namespace indicators{


static uint64_t calculate_time_threshold(const uint64_t &timestamp){
    // time_t time_in_sec = timestamp/1000;
    // if(m_BWFS_vars.confg->mode == indicators::ind_mode_e::STATIC){
    //     return (time_in_sec + m_BWFS_vars.confg->time*60)*1000;
    // }else{
    //     struct tm time = *localtime(&time_in_sec);
    //     uint64_t counter = 0;
    //     while(counter < time.tm_min){
    //         counter += m_BWFS_vars.confg->time;
    //     }
    //     uint64_t dif_secs = (counter - time.tm_min)*60;
    //     return (time_in_sec + dif_secs)*1000;
    // }
    return 0;
}

void bwfs_hndlr_callback( const backtesting::trade_data_t &trade_data, 
                          void *handler_){
    bwfs_hndlr_t *handler = (bwfs_hndlr_t*)handler_;
    std::cout<<__func__<<std::endl;

    // ind_db_itr_t itr = m_bwfs_ind_hndlr->process(trade_data, new_data);
    // if(m_BWFS_vars.set_threshold == true){
    //     m_BWFS_vars.time_threshold = calculate_time_threshold_(trade_data.eventTime);
    //     m_BWFS_vars.set_threshold = false;
    // }else{
    //     if(trade_data.eventTime > m_BWFS_vars.time_threshold){
    //         if(m_BWFS_vars.confg->mode == indicators::ind_mode_e::STATIC){
    //             {
    //                 indicators::indicators_list_t empty_list;
    //                 std::swap(itr->second, empty_list);
    //             }
    //             itr->second.push(new_data);
    //             m_BWFS_vars.time_threshold += m_BWFS_vars.confg->time*60*1000;
    //         }else{
    //             itr->second.pop();
    //             itr->second.push(new_data);
    //         }
    //     }
    // }
}
 
void bwfs_hndlr_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}