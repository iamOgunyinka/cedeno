#include <iostream>
#include "indicator_manager.hpp"

#include "BWFS/ticks_in.hpp"
#include "BWFS/ticks_out.hpp"
#include "BWFS/qty_in.hpp"
#include "BWFS/qty_out.hpp"
#include "BWFS/avrg_in.hpp"
#include "BWFS/avrg_out.hpp"
#include "BWFS/qty_in_out.hpp"
#include "BWFS/ticks_in_out.hpp"

using indicators::indicators_e;

static indicators::ind_db_t ind_db;
static indicators::c_indicators<backtesting::trade_data_t> ind_mngr;
static indicators::indicators_config_t ind_cfg;

static void init_indicators(void){
    typedef void (*ind_hndlr_t)(indicators::indicators_list_t &, indicators::indicator_data_t &, const backtesting::trade_data_t &);
    ind_hndlr_t ind_handlers[(uint64_t)indicators_e::SIZE] = {
    /*TICK_IN*/     indicators::ticks_in,
    /*TICK_OUT*/    indicators::ticks_out,
    /*QTY_IN*/      indicators::qty_in,
    /*QTY_OUT*/     indicators::qty_out,
    /*AVRG_IN*/     indicators::avrg_in,
    /*AVRG_OUT*/    indicators::avrg_out,
    /*QTY_IN_OUT*/  indicators::qty_in_out,
    /*TICK_IN_OUT*/ indicators::ticks_in_out
    };
    ind_mngr.init((uint64_t)indicators_e::SIZE, ind_handlers, &ind_db, &ind_cfg);  
}

void enable_all_indicators(void){
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::TICK_IN,       indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::TICK_OUT,      indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::QTY_IN,        indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::QTY_OUT,       indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::AVRG_IN,       indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::AVRG_OUT,      indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::QTY_IN_OUT,    indicators::ind_st_e::ENABLE);
    ind_mngr.modify_indicator_state((uint64_t)indicators_e::TICK_IN_OUT,   indicators::ind_st_e::ENABLE);
}

int main(void){
    init_indicators();
    enable_all_indicators();
    backtesting::trade_data_t new_tick;
    ind_mngr.process(new_tick);
    return 0;
}
