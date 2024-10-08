#ifndef INDC_DATA_HPP_
#define INDC_DATA_HPP_

#include <iostream>
#include <queue>
#include <unordered_map>
#include <memory>
#include "source_data.hpp"

namespace indicators{

enum class types_e{
    BUY_VS_SELL,
    TICK_IN,
    TICK_OUT,
    QTY_IN,
    QTY_OUT,
    AVRG_IN,
    AVRG_OUT,
    QTY_IN_OUT,
    TICK_IN_OUT,
    EMA,
    SMA,
    MACD,
    WMA,
    ATR,
    SAR,
    SIZE,
};


enum class source_e{
    SRC_TRADE,
    SRC_KLINE,
    SIZE,
};

struct ticks_in_t;
struct ticks_out_t;
struct qty_in_t;
struct qty_out_t;
struct avrg_in_t;
struct avrg_out_t;
struct qty_in_out_t;
struct ticks_in_out_t;
struct buy_vs_sell_t;
struct ema_t;
struct sma_t;
struct macd_t;
struct atr_t;
struct sar_t;
struct wma_t;

struct inf_BWFS_t{
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

struct inf_ema_t{
    double price = 0.0;
};

struct inf_sma_t{
    double price = 0.0;
};

struct inf_macd_t{
    double price = 0.0;
};

struct inf_wma_t{
    double price = 0.0;
};

struct inf_atr_t{
    double price = 0.0;
};

struct inf_sar_t{
    double price_up = 0.0;
    double price_down = 0.0;
    bool status = false;
};

struct inf_t{
    inf_BWFS_t cab; 
    inf_ema_t ema;
    inf_sma_t sma;
    inf_macd_t macd;
    inf_wma_t wma;
    inf_atr_t atr;
    inf_sar_t sar;
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
    std::unique_ptr<ema_t> ema_vars = nullptr;
    std::unique_ptr<sma_t> sma_vars = nullptr;
    std::unique_ptr<macd_t> macd_vars = nullptr;
    std::unique_ptr<wma_t> wma_vars = nullptr;
    std::unique_ptr<atr_t> atr_vars = nullptr;
    std::unique_ptr<sar_t> sar_vars = nullptr;
};

struct indicator_t{
    inf_t info;
    indcs_vars_t indcs_var;
};


static std::unordered_map<std::string, uint64_t> indc_list_key_string = {
{"tick_in",     (uint64_t)types_e::TICK_IN},
{"tick_out",    (uint64_t)types_e::TICK_OUT},
{"qty_in",      (uint64_t)types_e::QTY_IN},
{"qty_out",     (uint64_t)types_e::QTY_OUT},
{"avrg_in",     (uint64_t)types_e::AVRG_IN},
{"avrg_out",    (uint64_t)types_e::AVRG_OUT},
{"qty_in_out",  (uint64_t)types_e::QTY_IN_OUT},
{"tick_in_out", (uint64_t)types_e::TICK_IN_OUT},
{"buy_vs_sell", (uint64_t)types_e::BUY_VS_SELL},
{"ema",         (uint64_t)types_e::EMA},
{"sma",         (uint64_t)types_e::SMA},
{"macd",        (uint64_t)types_e::MACD},
{"wma",         (uint64_t)types_e::WMA},
{"atr",         (uint64_t)types_e::ATR},
{"sar",         (uint64_t)types_e::SAR}
};

static std::unordered_map<uint64_t, std::string> indc_list_key_number = {
{(uint64_t)types_e::TICK_IN,        "tick_in"},
{(uint64_t)types_e::TICK_OUT,       "tick_out"   },
{(uint64_t)types_e::QTY_IN,         "qty_in"     },
{(uint64_t)types_e::QTY_OUT,        "qty_out"    },
{(uint64_t)types_e::AVRG_IN,        "avrg_in"    },
{(uint64_t)types_e::AVRG_OUT,       "avrg_out"   },
{(uint64_t)types_e::QTY_IN_OUT,     "qty_in_out" },
{(uint64_t)types_e::TICK_IN_OUT,    "tick_in_out"},
{(uint64_t)types_e::BUY_VS_SELL,    "buy_vs_sell"},
{(uint64_t)types_e::EMA,            "ema"        },
{(uint64_t)types_e::SMA,            "sma"        },
{(uint64_t)types_e::MACD,           "macd"       },
{(uint64_t)types_e::WMA,            "wma"        },
{(uint64_t)types_e::ATR,            "atr"        },
{(uint64_t)types_e::SAR,            "sar"        }
};


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

#include "indicators/ema.hpp"
#include "indicators/sma.hpp"
#include "indicators/macd.hpp"
#include "indicators/wma.hpp"
#include "indicators/atr.hpp"
#include "indicators/sar.hpp"

#endif