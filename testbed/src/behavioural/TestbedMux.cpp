#include "./TestbedMux.h"
#include <string>

TestbedMux::TestbedMux(sc_module_name nm , int inPortSize, pfp::core::PFPObject* parent, std::string configfile):TestbedMuxSIM(nm ,inPortSize,parent,configfile) {  // NOLINT
    pcapLogger = new PcapLogger("ingress_sctime.pcap");
    packetCount = 0;
    /*sc_spawn threads*/
    for (size_t index = 0; index < inPortSize; index++) {
      ThreadHandles.push_back(sc_spawn(sc_bind
        (&TestbedMux::TestbedMux_PortServiceThread, this, index)));
    }
    ThreadHandles.push_back(sc_spawn(sc_bind
      (&TestbedMux::TestbedMuxThread, this, 0)));
    ThreadHandles.push_back(sc_spawn(sc_bind
      (&TestbedMux::packetLoop_thread, this)));
}
void TestbedMux::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TestbedMux::TestbedMux_PortServiceThread(std::size_t port_num) {
  // Thread function to service input ports.
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(in[port_num]->get());
    muxLock.lock();
    packetCount++;
    pcapLogger->logPacket(packet->getData(), sc_time_stamp());
    incomingPackets.push(packet);
    npulog(profile, cout << packetCount << " packets sent to NPU" << endl;)
    muxLock.unlock();
  }
}
void TestbedMux::TestbedMuxThread(std::size_t thread_id) {
  // Thread function for module functionalty
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<pfp::core::TrType> packet = incomingPackets.pop();
    timeval cpTime;
    gettimeofday(&cpTime, NULL);
    // BypassPacket bp(0, sc_time_stamp(), cpTime);
    // bypass->put(&bp);
    if (!out->nb_can_put()) {
      npulog(profile, cout << "Stuck at NPU Ingress! This is bad! Logical"
      << " Time: " << sc_time_stamp() << endl;)
      gotStuck = true;
    }
    out->put(packet);
    if (gotStuck) {
      npulog(profile, cout << "Resumed packet flow to ingress at logical "
      << "time: " << sc_time_stamp() << endl;)
      gotStuck = false;
    }
  }
}
void TestbedMux::packetLoop_thread() {
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(loop_in->get());
    muxLock.lock();
    pcapLogger->logPacket(packet->getData(), sc_time_stamp());
    incomingPackets.push(packet);
    muxLock.unlock();
  }
}
