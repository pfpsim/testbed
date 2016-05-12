#ifndef BEHAVIOURAL_TESTBEDDEMUX_H_
#define BEHAVIOURAL_TESTBEDDEMUX_H_
#include <string>
#include <vector>
#include "../structural/TestbedDemuxSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"

class TestbedDemux: public TestbedDemuxSIM {
 public:
  SC_HAS_PROCESS(TestbedDemux);
  /*Constructor*/
  TestbedDemux(sc_module_name nm , int outPortSize , pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~TestbedDemux() = default;

 public:
  void init();

 private:
  void TestbedDemux_PortServiceThread();
  void TestbedDemuxThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  void analyzeMetrics();
  void processPacketStream();
  void reinsertPacket(std::shared_ptr<TestbedPacket> packet);
};

#endif  // BEHAVIOURAL_TESTBEDDEMUX_H_
