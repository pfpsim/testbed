#include "./Mem.h"
#include <string>

Mem::Mem(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):MemSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
}

void Mem::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
