#include "./Testbed.h"
#include <string>

Testbed::Testbed(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):TestbedSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
}

void Testbed::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void Testbed::Testbed_PortServiceThread() {
  // Thread function to service input ports.
}
void Testbed::TestbedThread(std::size_t thread_id) {
  // Thread function for module functionalty
}
