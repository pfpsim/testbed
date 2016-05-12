#include "./Core.h"
#include <string>
#include <vector>

Core::Core(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):CoreSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
}

void Core::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
