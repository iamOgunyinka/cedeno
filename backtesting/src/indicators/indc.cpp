#include "indc.hpp"

namespace indicators{
indcs_c::indcs_c(indicators::indicator_t &common_db_){
    m_common_db = &common_db_;
}

indcs_c::~indcs_c(){

}
}