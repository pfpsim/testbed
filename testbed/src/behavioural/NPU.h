#ifndef BEHAVIOURAL_NPU_H_
#define BEHAVIOURAL_NPU_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/NPUSIM.h"

#include "pfpsim/core/cp/Commands.h"

class NPU: public NPUSIM {
 public:
  SC_HAS_PROCESS(NPU);
  /*Constructor*/
  explicit NPU(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~NPU() = default;

 public:
  void init();

 public:
  // For ControlPlaneAgentS
  std::shared_ptr<pfp::cp::CommandResult>
  send_command(const std::shared_ptr<pfp::cp::Command> & cmd) override;

 private:
  //! Internal list of submodules
  std::map<std::string, pfp::core::PFPObject*> common_interface_;
  // TODO(eric) test with debugger
};

#endif  // BEHAVIOURAL_NPU_H_
