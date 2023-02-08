#include "bwfs/bwfs_hndlr.hpp"

namespace indicators{


static uint64_t calculate_time_threshold(const indicators::ind_BWFS_confg_t &config, const uint64_t &timestamp){
    time_t time_now = timestamp/1000;
    uint64_t config_time = config.time*60;
    uint64_t start = time_now - time_now%config_time;
    return (start + config_time)*1000;
}

static void reset_indicator_datas_queue(bwfs_hndlr_t &handler){
    indicators::indicator_info_lis_t empty_list;
    std::swap(handler.indc_data_q, empty_list);
    (*handler.global_data).indc_info.cab = indicators::ind_BWFS_t();
}

void bwfs_hndlr_callback( const backtesting::trade_data_t &trade_data, 
                          indicator_t &handler_){
    bwfs_hndlr_t &handler = *handler_.indcs_var.bwfs_hndlr_vars;
    //std::cout<<__func__<<std::endl;

    if(handler.last_time_threshold != handler.configuration.time){
        handler.time_threshold = calculate_time_threshold(handler.configuration, trade_data.eventTime);
        // reset_indicator_datas_queue(handler);
        handler.indc_data_q.push((*handler.global_data).indc_info);
        handler.last_time_threshold =handler.configuration.time;
    }else{
        if(trade_data.eventTime > handler.time_threshold){
            if(handler.configuration.mode == indicators::ind_mode_e::STATIC){
                reset_indicator_datas_queue(handler);
                handler.indc_data_q.push((*handler.global_data).indc_info);
                handler.time_threshold = calculate_time_threshold(handler.configuration, trade_data.eventTime);
            }else{
                handler.indc_data_q.pop();
                handler.indc_data_q.push((*handler.global_data).indc_info);
            }
        }else{
            ///DELETE/////////////////
            if(handler.configuration.mode == indicators::ind_mode_e::STATIC){
                handler.indc_data_q.push((*handler.global_data).indc_info);
            }
        }
    }
}
 
void bwfs_hndlr_t::config(const indicators::ind_BWFS_confg_t &config_){
    configuration = config_;
}

}