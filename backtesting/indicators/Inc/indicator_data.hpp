#ifndef INDICATOR_DATA_HPP_
#define INDICATOR_DATA_HPP_
#include <iostream>
#include <queue>
#include <unordered_map>

namespace indicators{
    enum class indicators_e{
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
 
    typedef struct ind_confg_{
        ind_confg_(){
            candlestick_time = 0;
            ind_mode = ind_mode_e::STATIC;
        }
        uint64_t candlestick_time;
        ind_mode_e ind_mode;
    }ind_confg_t;

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
    typedef std::unordered_map<std::string, indicators::indicators_list_t> ind_db_t;
}
#endif