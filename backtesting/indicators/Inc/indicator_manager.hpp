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

template <typename T>
class ind_mngr_c{
    private:
        using  ind_list_t = indicators::indicators_list_t;
        using  ind_data_t = indicators::indicator_data_t;
        using ind_hndlr_p = void (*)(ind_list_t &, ind_data_t &, const T &);


        uint64_t m_num_ind;
        indicators::ind_db_t *m_ind_db;
        ind_hndlr_p (*m_ind_hndlr)[2];

        void *m_ind_confg;

        void set_indicator_handlers_(ind_hndlr_p ind_hndlr[]);
        void go_through_all_indicators_enable_(ind_list_t &ind_list, ind_data_t &new_data, const T &tick);

    public:
        ind_mngr_c();

        ind_mngr_c(   uint64_t num_ind, 
                        ind_hndlr_p ind_hndlr[],
                        indicators::ind_db_t *ind_db,
                        void *ind_confg,
                        uint64_t ind_confg_sz[]);

        void init(  uint64_t num_ind, 
                    ind_hndlr_p ind_hndlr[],
                    indicators::ind_db_t *ind_db,
                    void *ind_confg,
                    uint64_t ind_confg_sz[]);
                        
        ~ind_mngr_c();
        void process(const T &tick); 
        void modify_indicator_state(const uint64_t &indicator, const ind_st_e &new_state);
};

template <typename T>
void ind_mngr_c<T>::set_indicator_handlers_(ind_hndlr_p ind_hndlr[]){
    for(uint64_t index = 0; index < m_num_ind; index++){
        m_ind_hndlr[index][0] = nullptr;
        m_ind_hndlr[index][1] = ind_hndlr[index];
    }
}

template <typename T>
ind_mngr_c<T>::ind_mngr_c():m_ind_hndlr(nullptr){} 

template <typename T>
ind_mngr_c<T>::ind_mngr_c(  uint64_t num_ind, 
                                ind_hndlr_p ind_hndlr[],
                                indicators::ind_db_t *ind_db,
                                void *ind_confg,
                                uint64_t ind_confg_sz[]
                                ){
    init(num_ind, ind_hndlr, ind_db, ind_confg, ind_confg_sz);             
}

template <typename T>
ind_mngr_c<T>::~ind_mngr_c(){
}

template <typename T>
void ind_mngr_c<T>::init(uint64_t num_ind, 
                        ind_hndlr_p ind_hndlr[],
                        indicators::ind_db_t *ind_db,
                        void *ind_confg,
                        uint64_t ind_confg_sz[]){

    m_num_ind = num_ind;
    m_ind_db = ind_db;

    // m_ind_confg = (void*)new uint8_t[]; 
    
    m_ind_hndlr = new ind_hndlr_p[num_ind][2];
    set_indicator_handlers_(ind_hndlr);
}

template <typename T>
void ind_mngr_c<T>::modify_indicator_state(const uint64_t &ind, const ind_st_e &new_state){
    if(ind >= m_num_ind ){
        throw std::runtime_error("Wrong indicator number passed to enable");    
    }
    if(new_state == ind_st_e::DISABLE){
        m_ind_hndlr[ind][0] == nullptr;    
    }else{
        m_ind_hndlr[ind][0] = m_ind_hndlr[ind][1];  
    }
}

template <typename T>
void ind_mngr_c<T>::go_through_all_indicators_enable_(ind_list_t &ind_list, ind_data_t &new_data, const T &tick){
    for(uint64_t index = 0; index < m_num_ind; index++){
        if(m_ind_hndlr[index][0] != nullptr){
            (*m_ind_hndlr[index][0])(ind_list, new_data, tick);
        }
    }
}
template <typename T>
void ind_mngr_c<T>::process(const T &tick){
    ind_data_t new_data;
    auto itr = m_ind_db->find(tick.tokenName);
    ind_list_t *indicator_list;

    if(itr == m_ind_db->end()){
        ind_list_t ind_list_aux;
        ind_data_t first_empty_data; 
        ind_list_aux.push(first_empty_data);
        (*m_ind_db)[tick.tokenName] = ind_list_aux;

        go_through_all_indicators_enable_(ind_list_aux, new_data, tick);
        indicator_list = &(*m_ind_db)[tick.tokenName];
    }else{
        go_through_all_indicators_enable_(itr->second, new_data, tick);
        indicator_list = &itr->second;
    }

}

}
#endif