#include "./NPU.h"
#include <string>
#include "common/TlmVar.h"
#include "common/tlmsingleton.h"

#include "pfpsim/core/cp/Commands.h"

NPU::NPU(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):NPUSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
    tlmvar* tlm_var = new tlmvar();
    tlmsingelton::getInstance().tlmvarptr = tlm_var;
}

void NPU::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}

std::shared_ptr<pfp::cp::CommandResult>
NPU::send_command(const std::shared_ptr<pfp::cp::Command> & cmd) {
  return cpagent->send_command(cmd);
}
