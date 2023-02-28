#include "indicators/indicator.hpp"

namespace indicators{
indcs_c::indcs_c(indicator_t &common_db_){
    m_common_db = &common_db_;
}

indcs_c::~indcs_c(){

}
}