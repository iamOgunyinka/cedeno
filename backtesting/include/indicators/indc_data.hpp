#ifndef INDC_DATA_HPP_
#define INDC_DATA_HPP_

#include <iostream>
#include <queue>
#include <unordered_map>
#include <memory>

namespace indicators{

    struct ticks_in_t;
    struct ticks_out_t;
    struct qty_in_t;
    struct qty_out_t;
    struct avrg_in_t;
    struct avrg_out_t;
    struct qty_in_out_t;
    struct ticks_in_out_t;
    struct bwfs_hndlr_t;
    struct buy_vs_sell_t;

    enum class bwfs_inds_e{
        TICK_IN,
        TICK_OUT,
        QTY_IN,
        QTY_OUT,
        AVRG_IN,
        AVRG_OUT,
        QTY_IN_OUT,
        TICK_IN_OUT,
        SIZE,
    };

    enum class inds_e{
        TICK_IN,
        TICK_OUT,
        QTY_IN,
        QTY_OUT,
        AVRG_IN,
        AVRG_OUT,
        QTY_IN_OUT,
        TICK_IN_OUT,
        BUY_VS_SELL,
        BWFS_HANDLER,
        SIZE,
    };
    
    enum class ind_st_e{
        DISABLE,
        ENABLE,
    };

    enum class ind_mode_e{
        STATIC,
        DYNAMIC,
    };
 
    typedef struct ind_BWFS_confg_{
        ind_mode_e mode = ind_mode_e::STATIC;
        uint64_t time = 1;
        double client_confirmation = 0.0;
    }ind_BWFS_confg_t;

    struct ind_BWFS_t{
        uint64_t ticks_in = 0;
        uint64_t ticks_out = 0;
        double qty_in = 0.0;
        double qty_out = 0.0;
        double avrg_in = 0.0;
        double avrg_out = 0.0;
        int64_t ticks_in_out = 0;
        double qty_in_out = 0.0;
        double buyer_vs_seller = 0.0;
    };

    struct indcs_vars_t{
        std::unique_ptr<ticks_in_t> ticks_in_vars = nullptr; 
        std::unique_ptr<ticks_out_t> ticks_out_vars = nullptr; 
        std::unique_ptr<qty_in_t> qtys_in_vars = nullptr; 
        std::unique_ptr<qty_out_t> qty_out_vars = nullptr; 
        std::unique_ptr<avrg_in_t> avrg_in_vars = nullptr; 
        std::unique_ptr<avrg_out_t> avrg_out_vars = nullptr; 
        std::unique_ptr<qty_in_out_t> qty_in_out_vars = nullptr; 
        std::unique_ptr<ticks_in_out_t> ticks_in_out_vars = nullptr; 
        std::unique_ptr<buy_vs_sell_t> buy_vs_sell_vars = nullptr;
    };
    
    struct indicator_info_t{
        ind_BWFS_t cab; 
    };
    
    struct indicator_t{
        indicator_info_t indc_info;
        indcs_vars_t indcs_var;
    };

    using indicator_info_lis_t = std::queue<indicator_info_t>; 
}

#include "indicators/bwfs/ticks_in.hpp"
#include "indicators/bwfs/ticks_out.hpp"
#include "indicators/bwfs/qty_in.hpp"
#include "indicators/bwfs/qty_out.hpp"
#include "indicators/bwfs/avrg_in.hpp"
#include "indicators/bwfs/avrg_out.hpp"
#include "indicators/bwfs/qty_in_out.hpp"
#include "indicators/bwfs/ticks_in_out.hpp"
#include "indicators/bwfs/buy_vs_sell.hpp"

#endif