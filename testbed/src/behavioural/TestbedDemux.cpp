#include "./TestbedDemux.h"
#include <string>

TestbedDemux::TestbedDemux(sc_module_name nm , int outPortSize, pfp::core::PFPObject* parent, std::string configfile):TestbedDemuxSIM(nm ,outPortSize,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TestbedDemux::processPacketStream, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TestbedDemux::analyzeMetrics, this)));
}
void TestbedDemux::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TestbedDemux::TestbedDemux_PortServiceThread() {
  // Thread function to service input ports.
}
void TestbedDemux::TestbedDemuxThread(std::size_t thread_id) {
  // Thread function for module functionalty
}
void TestbedDemux::processPacketStream() {
  while (true) {
    std::shared_ptr<TestbedPacket> outgoing =
     std::dynamic_pointer_cast<TestbedPacket>(in->get());
    size_t portNumber = outgoing->getEgressPort() - 1;
    if (portNumber >= out.size()) {
      npulog(profile, cout << "FATAL: Going for port number: "
      << portNumber << endl;)
      // reinsertPacket(outgoing);
      assert(false);
    } else {
      npulog(profile, cout << "Going for port number: " << portNumber
      << endl;)
      out[portNumber]->put(outgoing);
    }
  }
}
void TestbedDemux::analyzeMetrics() {
  while (true) {
    std::shared_ptr<pfp::core::TrType> bp = bypass->get();
    // delete(bp);
  }
}
void TestbedDemux::reinsertPacket(std::shared_ptr<TestbedPacket> packet) {
  loop_out->put(packet);
}
