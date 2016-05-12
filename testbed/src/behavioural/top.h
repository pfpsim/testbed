#ifndef BEHAVIOURAL_TOP_H_
#define BEHAVIOURAL_TOP_H_
#include <string>
#include <vector>
#include "../structural/topSIM.h"

class top: public topSIM,  public DebuggerUtilities {
 public:
  SC_HAS_PROCESS(top);
  /*Constructor*/
  explicit top(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~top();

 public:
  void init();

 private:
  std::vector<sc_process_handle> ThreadHandles;
};

#endif  // BEHAVIOURAL_TOP_H_
