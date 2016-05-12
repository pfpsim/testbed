#ifndef BEHAVIOURAL_PACKETSINK_H_
#define BEHAVIOURAL_PACKETSINK_H_
#include <string>
#include <vector>
#include "../structural/PacketSinkSIM.h"

class PacketSink: public PacketSinkSIM {
 public:
  SC_HAS_PROCESS(PacketSink);
  /*Constructor*/
  explicit PacketSink(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~PacketSink() = default;

 public:
  void init();

 private:
  void PacketSink_PortServiceThread();
};

#endif  // BEHAVIOURAL_PACKETSINK_H_
