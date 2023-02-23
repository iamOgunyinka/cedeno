#ifndef INDC_HPP_
#define INDC_HPP_

namespace indicators{
struct indicator_t;
struct indcs_c{
    indcs_c(indicator_t &common_db_);
    ~indcs_c();
    indicator_t *m_common_db;
};
}
#include "indicators/bwfs/bwfs.hpp"

#endif