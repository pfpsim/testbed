#ifndef BEHAVIOURAL_TESTBEDMUX_H_
#define BEHAVIOURAL_TESTBEDMUX_H_
#include <string>
#include <vector>
#include "../structural/TestbedMuxSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class TestbedMux: public TestbedMuxSIM {
 public:
  SC_HAS_PROCESS(TestbedMux);
  /*Constructor*/
  TestbedMux(sc_module_name nm , int inPortSize , pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~TestbedMux() = default;

 public:
  void init();

 private:
  void TestbedMux_PortServiceThread(std::size_t port_num);
  void TestbedMuxThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  void BypassNPU(std::shared_ptr<pfp::core::TrType> inputS);
  void packetLoop_thread();

 private:
  MTQueue<std::shared_ptr<pfp::core::TrType> > incomingPackets;
  sc_mutex muxLock;
  PcapLogger *pcapLogger;
  uint64_t packetCount;
};

#endif  // BEHAVIOURAL_TESTBEDMUX_H_
