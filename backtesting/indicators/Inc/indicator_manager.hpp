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
#include "BWFS/qty_in.hpp"


namespace indicators{

template <typename T>
class ind_mngr_c{
    private:
        using  ind_list_t = indicators::indicators_list_t;
        using  ind_data_t = indicators::indicator_data_t; 
        using ind_hndlr_p = void (*)(const T &, void*);

        uint64_t m_num_indcs;
        uint64_t m_num_indcs_set;
        ind_hndlr_p *m_call_backs;
        intptr_t *m_callBack_data;

        void set_indicator_handlers_(ind_hndlr_p ind_hndlr[]);

    public:
        ind_mngr_c();

        ind_mngr_c(const uint64_t &num_ind);

        void add_indicator(ind_hndlr_p ind_hndlr, const intptr_t &data);
                        
        ~ind_mngr_c();
        void process(const T &tick);  
};

template <typename T>
void ind_mngr_c<T>::set_indicator_handlers_(ind_hndlr_p ind_hndlr[]){
    for(uint64_t index = 0; index < m_num_indcs; index++){
        m_call_backs[index] = ind_hndlr[index];
    }
}

template <typename T>
ind_mngr_c<T>::ind_mngr_c():m_call_backs(nullptr),
                            m_callBack_data(nullptr),
                            m_num_indcs_set(0){} 

template <typename T>
ind_mngr_c<T>::ind_mngr_c(const uint64_t &num_indc): m_num_indcs(num_indc),
                                                     m_num_indcs_set(0){
    m_call_backs = new ind_hndlr_p[num_indc];
    m_callBack_data = new intptr_t[num_indc];
} 

template <typename T>
ind_mngr_c<T>::~ind_mngr_c(){
}

template <typename T>
void ind_mngr_c<T>::add_indicator(ind_hndlr_p ind_hndlr, const intptr_t &data){
    if(m_num_indcs_set == m_num_indcs){
        std::__throw_runtime_error("Overflow in indicator manager");
    }
    m_call_backs[m_num_indcs_set] = ind_hndlr;
    m_callBack_data[m_num_indcs_set] = data;
    m_num_indcs_set++;
}

template <typename T>
void ind_mngr_c<T>::process(const T &data){
    for(uint64_t index = 0; index < m_num_indcs_set; index++){
        (*m_call_backs[index])(data, (void*)m_callBack_data[index]);
    }
}

}
#endif