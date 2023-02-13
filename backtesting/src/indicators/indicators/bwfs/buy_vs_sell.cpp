#include "indicators/bwfs/buy_vs_sell.hpp"
namespace indicators{

static uint64_t calculate_time_threshold( const indicators::ind_BWFS_confg_t &config, 
                                          const uint64_t &timestamp){

    time_t time_now = timestamp/1000;
    uint64_t config_time = config.time*60;
    uint64_t start = time_now - time_now%config_time;
    return (start + config_time)*1000;
}

static void reset_indicator_datas_queue(buy_vs_sell_t &handler){
    std::queue<buy_vs_sell_q_t> empty_list;
    std::swap(handler.price_q, empty_list);
    (*handler.common_db).indc_info.cab = indicators::ind_BWFS_t();
}

static void decrease_quantity( buy_vs_sell_t &handler, 
                               const double &price, 
                               const std::string &buyer_id, 
                               const std::string &seller_id){

    double &buyer_vs_seller = handler.common_db->indc_info.cab.buyer_vs_seller;
    double &clnt_conf = handler.configuration->client_confirmation;

    double last_price_buyer = handler.buyer[buyer_id];
    double current_price_buyer = last_price_buyer - price;
    if(current_price_buyer < clnt_conf && last_price_buyer >= clnt_conf){
        buyer_vs_seller -= last_price_buyer;
    }else if(current_price_buyer >= clnt_conf && last_price_buyer >= clnt_conf){
        buyer_vs_seller -= last_price_buyer - current_price_buyer;        
    }
    handler.buyer[buyer_id] = current_price_buyer; 

    double last_price_seller = handler.buyer[seller_id];
    double current_price_seller = last_price_seller - price;
    if(current_price_seller < clnt_conf && last_price_seller >= clnt_conf){
        buyer_vs_seller += last_price_seller;
    }else if(current_price_seller >= clnt_conf && last_price_seller >= clnt_conf){
        buyer_vs_seller += last_price_seller - current_price_seller;        
    }
    handler.seller[seller_id] = current_price_seller; 
}

static void increase_quantity( buy_vs_sell_t &handler, 
                               const double &price, 
                               const std::string &buyer_id, 
                               const std::string &seller_id){

    double &buyer_vs_seller = handler.common_db->indc_info.cab.buyer_vs_seller;
    double &clnt_conf = handler.configuration->client_confirmation;

    double last_price_buyer = handler.buyer[buyer_id];
    double current_price_buyer = last_price_buyer + price;
    if(last_price_buyer < clnt_conf && current_price_buyer >= clnt_conf){
        buyer_vs_seller += current_price_buyer;
    }else if(last_price_buyer >= clnt_conf && current_price_buyer >= clnt_conf){
        buyer_vs_seller += current_price_buyer - last_price_buyer;        
    }
    handler.buyer[buyer_id] = current_price_buyer; 

    double last_price_seller = handler.seller[seller_id];
    double current_price_seller = last_price_seller + price;
    if(last_price_seller < clnt_conf && current_price_seller >= clnt_conf){
        buyer_vs_seller -= current_price_seller;
    }else if(last_price_seller >= clnt_conf && current_price_seller >= clnt_conf){
        buyer_vs_seller -= current_price_seller - last_price_seller;        
    }
    handler.seller[seller_id] = current_price_seller; 
}

void buy_vs_sell_callback( const backtesting::trade_data_t &trade_data, 
                          indicator_t &handler_){

    buy_vs_sell_t &handler = *handler_.indcs_var.buy_vs_sell_vars;
    std::cout<<__func__<<std::endl;

    if(handler.last_time_threshold != handler.configuration->time){
        reset_indicator_datas_queue(handler);
        handler.time_threshold = calculate_time_threshold( *handler.configuration, 
                                                           trade_data.eventTime);
        increase_quantity( handler, trade_data.quantityExecuted, 
                           "buyer_id", "seller_id");
        handler.price_q.push(buy_vs_sell_q_t( trade_data.quantityExecuted, 
                                              "buyer_id", 
                                              "seller_id"));
        handler.last_time_threshold = handler.configuration->time;
    }else{
        if(trade_data.eventTime > handler.time_threshold){
            if(handler.configuration->mode == indicators::ind_mode_e::STATIC){
                reset_indicator_datas_queue(handler);
                handler.time_threshold = calculate_time_threshold( *handler.configuration, 
                                                                   trade_data.eventTime);
            }else{
                const buy_vs_sell_q_t &back = handler.price_q.back();
                increase_quantity( handler, trade_data.quantityExecuted, 
                                   "buyer_id", "seller_id");
                decrease_quantity(handler, back.quantity, back.buyer_id, back.seller_id);
                handler.price_q.pop();
                handler.price_q.push(buy_vs_sell_q_t( trade_data.quantityExecuted, 
                                                      "buyer_id", 
                                                      "seller_id"));
            }
        }else{
            if(handler.configuration->mode == indicators::ind_mode_e::DYNAMIC){
                increase_quantity( handler, trade_data.quantityExecuted, 
                                   "buyer_id", "seller_id");
                handler.price_q.push(buy_vs_sell_q_t( trade_data.quantityExecuted, 
                                                      "buyer_id", 
                                                      "seller_id"));
            }
        }
    }
}

}