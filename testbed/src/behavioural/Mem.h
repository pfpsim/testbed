#ifndef BEHAVIOURAL_MEM_H_
#define BEHAVIOURAL_MEM_H_
#include <string>
#include <vector>
#include "../structural/MemSIM.h"

class Mem: public MemSIM {
 public:
  SC_HAS_PROCESS(Mem);
  /*Constructor*/
  explicit Mem(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Mem() = default;

 public:
  void init();
};

#endif  // BEHAVIOURAL_MEM_H_
