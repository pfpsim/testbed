#include "./LoadBalancer.h"
#include <string>

LoadBalancer::LoadBalancer(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):LoadBalancerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind(&LoadBalancer::temp1, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind(&LoadBalancer::temp2, this)));
}

void LoadBalancer::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
}
void LoadBalancer::LoadBalancer_PortServiceThread() {
  // Thread function to service input ports.
}
void LoadBalancer::LoadBalancerThread(std::size_t thread_id) {
  // Thread function for module functionalty
}

void LoadBalancer::temp1() {
  while (true) {
    out_cl->put(in_cl->get());
  }
}

void LoadBalancer::temp2() {
  while (true) {
    out_se->put(in_se->get());
  }
}

// Port service methods
void LoadBalancer::outgoingPackets_thread() {
}
void LoadBalancer::validatePacketDestination_thread() {
}

// Administrative methods
void LoadBalancer::populateLocalMap() {
}
void LoadBalancer::mapAllServers() {
}
void LoadBalancer::addServerInstance_thread() {
}
void LoadBalancer::scheduler_thread() {
}

// Behavioral methods
void establishServer(std::string clientID) {
}
