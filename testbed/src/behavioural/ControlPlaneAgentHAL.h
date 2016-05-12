#ifndef BEHAVIOURAL_CONTROLPLANEAGENTHAL_H_
#define BEHAVIOURAL_CONTROLPLANEAGENTHAL_H_
#include <string>
#include <vector>
#include "../structural/ControlPlaneAgentHALSIM.h"
#include "CommonIncludes.h"

class ControlPlaneAgentHAL:
  public ControlPlaneAgentHALSIM,
  public MemoryUtilities,
  public MemoryMaps {
 public:
  SC_HAS_PROCESS(ControlPlaneAgentHAL);
  /*Constructor*/
  explicit ControlPlaneAgentHAL(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~ControlPlaneAgentHAL() = default;

 public:
  void init();

  virtual void tlmwrite(int VirtualAddress, int data, TlmType size);
  virtual TlmType tlmread(int VirtualAddress);
  virtual TlmType tlmallocate(int BytestoAllocate);
};

#endif  // BEHAVIOURAL_CONTROLPLANEAGENTHAL_H_
