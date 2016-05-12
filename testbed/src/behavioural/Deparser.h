#ifndef BEHAVIOURAL_DEPARSER_H_
#define BEHAVIOURAL_DEPARSER_H_
#include <string>
#include <vector>
#include <map>
#include <queue>
#include "../structural/DeparserSIM.h"
#include "common/Packet.h"
#include "common/TestbedPacket.h"
#include "common/PacketDescriptor.h"

class Deparser: public DeparserSIM {
 public:
  SC_HAS_PROCESS(Deparser);
  /*Constructor*/
  explicit Deparser(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Deparser() = default;

 public:
  void init();

 private:
  void Deparser_PortServiceThread();
  void DeparserThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  std::map<std::size_t, std::shared_ptr<PacketDescriptor>> memory_;
  std::ofstream outlog;
  void DeparserThread_QueueService(std::size_t thread_id);
  std::queue<std::shared_ptr<PacketDescriptor>> fromTrafficManagerBuffer;
  std::queue<std::shared_ptr<Packet>> fromMEM;

  sc_mutex tmguard;
  sc_mutex memguard;
  sc_event bufferevent;
};

#endif  // BEHAVIOURAL_DEPARSER_H_
