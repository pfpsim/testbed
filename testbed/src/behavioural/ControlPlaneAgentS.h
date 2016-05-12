#ifndef BEHAVIOURAL_CONTROLPLANEAGENTS_H_
#define BEHAVIOURAL_CONTROLPLANEAGENTS_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

#include "pfpsim/core/cp/Commands.h"  // TODO(gordon) is this still necessary?

class ControlPlaneAgentS : public sc_interface {
 public:
  virtual std::shared_ptr<pfp::cp::CommandResult>
  send_command(const std::shared_ptr<pfp::cp::Command> & cmd) = 0;
};
#endif  // BEHAVIOURAL_CONTROLPLANEAGENTS_H_
