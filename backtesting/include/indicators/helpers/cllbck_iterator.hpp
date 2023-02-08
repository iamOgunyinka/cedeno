#ifndef CLLBCK_ITERATOR_HPP_
#define CLLBCK_ITERATOR_HPP_

#include "user_data.hpp"
#include "indc_data.hpp"

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
        using  ind_data_t = indicators::indicator_t; 
        using indc_cllbck_p = void (*)(const T &, indicators::indicator_t &);

        uint64_t m_sz_indcs_set = 0;
        uint64_t m_sz_indcs;
        indc_cllbck_p *m_cllbcks; 

    public:
        ind_mngr_c();

        ind_mngr_c(const uint64_t &indcs_sz);

        void init_indcs_sz(const uint64_t &indcs_sz);
        void add_indicator(indc_cllbck_p indc_callback);
                        
        ~ind_mngr_c();
        void process(const T &tick, indicator_t &indc_data);  
};


template <typename T>
ind_mngr_c<T>::ind_mngr_c():m_cllbcks(nullptr), 
                            m_sz_indcs(0){} 

template <typename T>
ind_mngr_c<T>::ind_mngr_c(const uint64_t &sz_indcs): m_sz_indcs(sz_indcs){
    m_cllbcks = new indc_cllbck_p[sz_indcs]{nullptr};
} 


template <typename T>
void ind_mngr_c<T>::init_indcs_sz(const uint64_t &sz_indcs){
    m_cllbcks = new indc_cllbck_p[sz_indcs];
    m_sz_indcs = sz_indcs;
}

template <typename T>
ind_mngr_c<T>::~ind_mngr_c(){
    if(m_cllbcks != nullptr)
        delete m_cllbcks;
}

template <typename T>
void ind_mngr_c<T>::add_indicator(indc_cllbck_p ind_hndlr){
    if(m_sz_indcs == 0){
        std::__throw_runtime_error("The indicator size has not beed set");
    }
    if(m_sz_indcs_set == m_sz_indcs){
        std::__throw_runtime_error("Overflow in indicator manager");
    }
    m_cllbcks[m_sz_indcs_set] = ind_hndlr;
    m_sz_indcs_set++;
}

template <typename T>
void ind_mngr_c<T>::process(const T &data, indicator_t &indc_data){
    for(uint64_t index = 0; index < m_sz_indcs_set; index++){
        (*m_cllbcks[index])(data, indc_data);
    }
}
}
#endif