#include "./top.h"
#include <string>

top::top(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):topSIM(nm, parent, configfile), DebuggerUtilities() {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
    ThreadHandles.push_back(
      sc_spawn(
        sc_bind(&top::notify_observers, this, this)));
}

top::~top() {
}

void top::init() {
#ifdef PFPSIM_DEBUGGER
  attach_observer(debug_obs);
  if (PFP_DEBUGGER_ENABLED) {
    debug_obs->enableDebugger();
    debug_obs->waitForRunCommand();
  }
#endif

#ifndef PFPSIM_DEBUGGER
  if (PFP_DEBUGGER_ENABLED) {
    std::cerr << "\n" << On_IRed
              << "ERROR: Please compile with PFPSIM_DEBUGGER flag for access to"
              << " the debugger. Use: cmake -DPFPSIMDEBUGGER=ON ../src/ in the"
              << " build directory." << txtrst << std::endl;
    return 1;
  }
#endif
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
