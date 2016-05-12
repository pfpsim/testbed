#include "./Cluster.h"
#include <string>

Cluster::Cluster(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ClusterSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
}

void Cluster::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
