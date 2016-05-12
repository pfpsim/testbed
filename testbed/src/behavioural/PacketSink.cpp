#include "./PacketSink.h"
#include <string>

PacketSink::PacketSink(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):PacketSinkSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
    sc_spawn(sc_bind(&PacketSink::PacketSink_PortServiceThread, this));
}

void PacketSink::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void PacketSink::PacketSink_PortServiceThread() {
  while (true) {
    in->get();
  }
}
