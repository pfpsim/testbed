#ifndef BEHAVIOURAL_TESTBED_H_
#define BEHAVIOURAL_TESTBED_H_
#include <string>
#include <vector>
#include "../structural/TestbedSIM.h"

class Testbed: public TestbedSIM {
 public:
  SC_HAS_PROCESS(Testbed);
  /*Constructor*/
  explicit Testbed(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Testbed() = default;

 public:
  void init();

 private:
  void Testbed_PortServiceThread();
  void TestbedThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;
};

#endif  // BEHAVIOURAL_TESTBED_H_
