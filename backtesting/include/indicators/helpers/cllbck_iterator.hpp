#ifndef CLLBCK_ITERATOR_HPP_
#define CLLBCK_ITERATOR_HPP_

#include <unordered_map>
#include <iostream>
#include <string>
#include <memory>
#include <bits/stdc++.h>


namespace indicators{

template <typename T, typename B>
class ind_mngr_c{
    private:
        using indc_cllbck_p = void (*)(const T &, B &);

        uint64_t m_sz_indcs_set = 0;
        uint64_t m_sz_indcs;
        indc_cllbck_p *m_cllbcks; 

    public:
        ind_mngr_c();

        ind_mngr_c(const uint64_t &indcs_sz);

        void init_indcs_sz(const uint64_t &indcs_sz);
        void add_indicator(indc_cllbck_p indc_callback);
                        
        ~ind_mngr_c();
        void process(const T &tick, B &indc_data);  
};


template <typename T,typename B>
ind_mngr_c<T, B>::ind_mngr_c():
    m_cllbcks(nullptr), 
    m_sz_indcs(0){} 

template <typename T,typename B>
ind_mngr_c<T, B>::ind_mngr_c(const uint64_t &sz_indcs): 
    m_sz_indcs(sz_indcs){
    m_cllbcks = new indc_cllbck_p[sz_indcs]{nullptr};
} 


template <typename T,typename B>
void ind_mngr_c<T, B>::init_indcs_sz(const uint64_t &sz_indcs){
    m_cllbcks = new indc_cllbck_p[sz_indcs];
    m_sz_indcs = sz_indcs;
}

template <typename T,typename B>
ind_mngr_c<T, B>::~ind_mngr_c(){
    if(m_cllbcks != nullptr)
        delete m_cllbcks;
}

template <typename T,typename B>
void ind_mngr_c<T, B>::add_indicator(indc_cllbck_p ind_hndlr){
    if(m_sz_indcs == 0){
        std::__throw_runtime_error("The indicator size has not beed set");
    }
    if(m_sz_indcs_set == m_sz_indcs){
        std::__throw_runtime_error("Overflow in indicator manager");
    }
    m_cllbcks[m_sz_indcs_set] = ind_hndlr;
    m_sz_indcs_set++;
}

template <typename T,typename B>
void ind_mngr_c<T, B>::process( const T &data, 
                                B &indc_data){
    for(uint64_t index = 0; index < m_sz_indcs_set; index++){
        (*m_cllbcks[index])(data, indc_data);
    }
}
}
#endif