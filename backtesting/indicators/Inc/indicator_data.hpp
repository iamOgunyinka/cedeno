#ifndef INDICATOR_DATA_HPP_
#define INDICATOR_DATA_HPP_
#include <iostream>
#include <queue>
#include <unordered_map>

namespace indicators{
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
        ind_BWFS_confg_(){
            mode = ind_mode_e::STATIC;
            time = 8;
            limit = 0.0;
        }
        ind_mode_e mode;
        uint64_t time;
        double limit;
    }ind_BWFS_confg_t;

    typedef struct ind_BWFS_{
        ind_BWFS_(){
            ticks_in = 0;
            ticks_out = 0;
            qty_in = 0.0;
            qty_out = 0.0;
            avrg_in = 0.0;
            avrg_out = 0.0;
            ticks_in_out = 0;
            qty_in_out = 0.0;
        }
        
        uint64_t ticks_in;
        uint64_t ticks_out;
        double qty_in;
        double qty_out;
        double avrg_in;
        double avrg_out;
        int64_t ticks_in_out;
        double qty_in_out;
    }ind_BWFS_t;
    
    typedef struct indictors_{
        ind_BWFS_t cab; 
    }indicator_data_t;

    using indicators_list_t = std::queue<indicator_data_t>; 
}
#endif