#include "indicators/bwfs/buy_vs_sell.hpp"
namespace indicators{

static uint64_t calculate_time_threshold( const conf_BWFS_t &config, 
                                          const uint64_t &timestamp){

    time_t time_now = timestamp/1000;
    uint64_t config_time = config.time*60;
    uint64_t start = time_now - time_now%config_time;
    return (start + config_time)*1000;
}

static void reset_info_data(buy_vs_sell_t &handler){
    std::queue<buy_vs_sell_q_t> empty_list;
    std::swap(handler.price_q, empty_list);
    (*handler.common_db).info.cab = inf_BWFS_t();
}

static auto get_user( std::unordered_map<std::string, double> &handler,
                         const std::string &id){
    auto itr = handler.find(id);
    if(itr == handler.end()){
        handler.emplace(id, 0.0);
        return handler.find(id);
    }
    return itr;
}

static void decrease_quantity( buy_vs_sell_t &handler, 
                               const double &quantity, 
                               const std::string &buyer_id, 
                               const std::string &seller_id){

    double &buyer_vs_seller = handler.common_db->info.cab.buyer_vs_seller;
    double &clnt_conf = handler.configuration->client_confirmation;

    double &last_quantity_buyer = get_user(handler.buyer, buyer_id)->second;
    double current_price_buyer = last_quantity_buyer - quantity;
    if(current_price_buyer < clnt_conf && last_quantity_buyer >= clnt_conf){
        buyer_vs_seller -= last_quantity_buyer;
    }else if(current_price_buyer >= clnt_conf){
        buyer_vs_seller -= quantity;        
    }
    last_quantity_buyer = current_price_buyer; 

    double &last_quantity_seller = get_user(handler.buyer, buyer_id)->second;
    double current_price_seller = last_quantity_seller - quantity;
    if(current_price_seller < clnt_conf && last_quantity_seller >= clnt_conf){
        buyer_vs_seller += last_quantity_seller;
    }else if(current_price_seller >= clnt_conf ){
        buyer_vs_seller += quantity;        
    }
    last_quantity_seller = current_price_seller; 
}

static void increase_quantity( buy_vs_sell_t &handler, 
                               const double &quantity, 
                               const std::string &buyer_id, 
                               const std::string &seller_id){

    double &buyer_vs_seller = handler.common_db->info.cab.buyer_vs_seller;
    double &clnt_conf = handler.configuration->client_confirmation;

    double &last_quantity_buyer = get_user(handler.buyer, buyer_id)->second;
    double current_price_buyer = last_quantity_buyer + quantity;
    if(last_quantity_buyer < clnt_conf && current_price_buyer >= clnt_conf){
        buyer_vs_seller += current_price_buyer;
    }else if(last_quantity_buyer >= clnt_conf){
        buyer_vs_seller += quantity;        
    }
    last_quantity_buyer = current_price_buyer; 

    double &last_quantity_seller = get_user(handler.buyer, buyer_id)->second;
    double current_price_seller = last_quantity_seller + quantity;
    if(last_quantity_seller < clnt_conf && current_price_seller >= clnt_conf){
        buyer_vs_seller -= current_price_seller;
    }else if(last_quantity_seller >= clnt_conf){
        buyer_vs_seller -= quantity;        
    }
    last_quantity_seller = current_price_seller; 
}

void buy_vs_sell_callback( const trade_stream_d &trade_data, 
                          indicator_t &handler_){

    buy_vs_sell_t &handler = *handler_.indcs_var.buy_vs_sell_vars;
    std::cout<<__func__<<std::endl;

    if(handler.last_time_threshold != handler.configuration->time){
        reset_info_data(handler);
        handler.time_threshold = calculate_time_threshold( *handler.configuration, 
                                                           trade_data.eventTimeMs);
        increase_quantity( handler, trade_data.quantity,
                           "buyer_id", "seller_id");
        handler.price_q.push(buy_vs_sell_q_t( trade_data.quantity,
                                              "buyer_id", 
                                              "seller_id"));
        handler.last_time_threshold = handler.configuration->time;
    }else{
        if(trade_data.eventTimeMs > handler.time_threshold){
            if(handler.configuration->mode == bwfs_mode_e::STATIC){
                reset_info_data(handler);
                handler.time_threshold = calculate_time_threshold( *handler.configuration,
                                                                   trade_data.eventTimeMs);
            }else{
                const buy_vs_sell_q_t &back = handler.price_q.back();
                increase_quantity( handler, trade_data.quantity,
                                   "buyer_id", "seller_id");
                decrease_quantity(handler, back.quantity, back.buyer_id, back.seller_id);
                handler.price_q.pop();
                handler.price_q.push(buy_vs_sell_q_t( trade_data.quantity,
                                                      "buyer_id", 
                                                      "seller_id"));
            }
        }else{
            if(handler.configuration->mode == bwfs_mode_e::DYNAMIC){
                increase_quantity( handler, trade_data.quantity,
                                   "buyer_id", "seller_id");
                handler.price_q.push(buy_vs_sell_q_t( trade_data.quantity,
                                                      "buyer_id", 
                                                      "seller_id")); 
            }
        }
    }
}

}