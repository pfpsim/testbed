#ifndef BEHAVIOURAL_CORE_H_
#define BEHAVIOURAL_CORE_H_
#include <string>
#include <vector>
#include "../structural/CoreSIM.h"

class Core: public CoreSIM {
 public:
  SC_HAS_PROCESS(Core);
  /*Constructor*/
  explicit Core(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Core() = default;

 public:
  void init();
};

#endif  // BEHAVIOURAL_CORE_H_
