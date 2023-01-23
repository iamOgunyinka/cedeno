#ifndef INDI_DATA_HPP_
#define INDI_DATA_HPP_
#include <iostream>
#include <unordered_map>
#include <string>

namespace indicators{
    typedef struct indictors_{
        indictors_(){
            ticks_in = 0;
            ticks_out = 0;
            qty_in = 0;
            qty_in = 0;
            avrg_out = 0;
            avrg_out = 0;
            ticks_in_out = 0;
            qty_in_out = 0;
        }

        uint64_t ticks_in;
        uint64_t ticks_out;
        uint64_t qty_in;
        uint64_t qty_in;
        uint64_t avrg_out;
        uint64_t avrg_out;
        uint64_t ticks_in_out;
        uint64_t qty_in_out;
    }indictors_t;
    
    std::unordered_map<std::string, indictors_t> data_base;
}
#endif