#ifndef INDICATOR_MANAGER_HPP_
#define INDICATOR_MANAGER_HPP_

#include "user_data.hpp"
#include "indicator_data.hpp"
#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm> 
#include <unordered_map> 
#include <bits/stdc++.h>

namespace indicators{

enum class ind_mode_e{
    STATIC,
    DYNAMIC,
};

enum class ind_st_e{
    DISABLE,
    ENABLE,
};

using  ind_list_t = indicators::indicators_list_t;
using  ind_data_t = indicators::indicator_data_t;

template <typename T>
class c_indicators{
    private:
        ind_mode_e ind_mode;
        uint64_t num_ind;
        indicators::ind_db_t *ind_db;
        static void indicator_disable_handler(  ind_list_t &ind_list, 
                                                ind_data_t &new_data, 
                                                const T &){std::cout<<"Disable"<<std::endl;}

        void set_indicator_handlers(void (*ind_hndlr[])(ind_list_t &, ind_data_t &, const T &));

        void set_indicator_states(bool *ind_state);
        void go_through_all_indicators_enable(ind_list_t &ind_list, ind_data_t &new_data, const T &tick);

        void (*(*ind_hndlr)[2])(ind_list_t &, ind_data_t &, const T &);
        bool *ind_state;
    public:
        c_indicators();

        c_indicators(   uint64_t num_ind, 
                        void (*ind_hndlr[])(ind_list_t &, ind_data_t &, const T &),
                        indicators::ind_db_t *ind_db);

        void init(   uint64_t num_ind, 
                void (*ind_hndlr[])(ind_list_t &, ind_data_t &, const T &),
                indicators::ind_db_t *ind_db);
                        
        ~c_indicators();
        void process(const T &tick); 
        void set_mode(const ind_mode_e &new_mode);
        void modify_indicator_state(const uint64_t &indicator, const ind_st_e &new_state);
};

template <typename T>
void c_indicators<T>::set_indicator_states(bool *ind_state){
    for(uint64_t index = 0; index < num_ind; index++){
        this->ind_state[index] = ind_state[index];
    }
}

template <typename T>
void c_indicators<T>::set_indicator_handlers(void (*ind_hndlr[])(ind_list_t &, ind_data_t &, const T &)){
    for(uint64_t index = 0; index < num_ind; index++){
        this->ind_hndlr[index][0] = c_indicators::indicator_disable_handler;
        this->ind_hndlr[index][1] = ind_hndlr[index];
    }
}

template <typename T>
c_indicators<T>::c_indicators(){
    ind_mode = ind_mode_e::STATIC;
    
    this->ind_state = new bool[num_ind];
    memset(this->ind_state, (uint8_t)false, num_ind);
} 

template <typename T>
c_indicators<T>::c_indicators(  uint64_t num_ind, 
                                void (*ind_hndlr[])(ind_list_t &, ind_data_t &, const T &),
                                indicators::ind_db_t *ind_db
                                ){
    init(num_ind, ind_hndlr, ind_db);             
}

template <typename T>
c_indicators<T>::~c_indicators(){
    // delete indicator_handler;
    // delete indicator_state;
}

template <typename T>
void c_indicators<T>::init(uint64_t num_ind, 
                        void (*ind_hndlr[])(ind_list_t &, ind_data_t &, const T &),
                        indicators::ind_db_t *ind_db){

    typedef void (*ind_hndlr_ptr)(ind_list_t &, ind_data_t &, const T &);

    this->num_ind = num_ind;
    ind_mode = ind_mode_e::STATIC;
    this->ind_db = ind_db;
    
    this->ind_hndlr = new ind_hndlr_ptr[num_ind][2];
    set_indicator_handlers(ind_hndlr);

    this->ind_state = new bool[num_ind];
    memset(this->ind_state, (uint8_t)false, num_ind);

}

template <typename T>
void c_indicators<T>::set_mode(const ind_mode_e &new_mode){
    ind_mode = new_mode;
}

template <typename T>
void c_indicators<T>::modify_indicator_state(const uint64_t &ind, const ind_st_e &new_state){
    if(ind >= num_ind ){
        throw std::runtime_error("Wrong indicator number passed to enable");    
    }
    ind_state[ind] = (bool)new_state;
}

template <typename T>
void c_indicators<T>::go_through_all_indicators_enable(ind_list_t &ind_list, ind_data_t &new_data, const T &tick){
    for(uint64_t index = 0; index < num_ind; index++){
        bool state = ind_state[index]; 
        (*ind_hndlr[index][(uint8_t)state])(ind_list, new_data, tick);
    }
}
template <typename T>
void c_indicators<T>::process(const T &tick){
    ind_data_t new_data;
    auto itr = ind_db->find(tick.tokenName);
    ind_list_t *indicator_list;

    if(itr == ind_db->end()){
        ind_list_t ind_list_aux;
        ind_data_t first_empty_data; 
        ind_list_aux.emplace(first_empty_data);
        (*ind_db)[tick.tokenName] = ind_list_aux;

        go_through_all_indicators_enable(ind_list_aux, new_data, tick);
        indicator_list = &(*ind_db)[tick.tokenName];
    }else{
        go_through_all_indicators_enable(itr->second, new_data, tick);
        indicator_list = &itr->second;
    }

    if(ind_mode == ind_mode_e::STATIC){
        indicator_list->push(new_data);
    }else{

    }
}

}
#endif