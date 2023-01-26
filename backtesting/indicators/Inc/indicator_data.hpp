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

    typedef struct creed_and_bear_{
        creed_and_bear_(){
            ticks_in = 0;
            ticks_out = 0;
            qty_in = 0.0;
            qty_out = 0.0;
            avrg_in = 0;
            avrg_out = 0;
            ticks_in_out = 0;
            qty_in_out = 0;
        }

        uint64_t ticks_in;
        uint64_t ticks_out;
        double qty_in;
        double qty_out;
        uint64_t avrg_in;
        uint64_t avrg_out;
        uint64_t ticks_in_out;
        uint64_t qty_in_out;
    }creed_and_bear_t;
    
    typedef struct indictors_{
        creed_and_bear_t cab; 
    }indicator_data_t;

    using indicators_list_t = std::queue<indicator_data_t>; 
    typedef std::unordered_map<std::string, indicators::indicators_list_t> ind_db_t;
}
#endif