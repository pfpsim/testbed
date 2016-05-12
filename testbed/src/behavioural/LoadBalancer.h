#ifndef BEHAVIOURAL_LOADBALANCER_H_
#define BEHAVIOURAL_LOADBALANCER_H_
#include <string>
#include <vector>
#include "../structural/LoadBalancerSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"

class LoadBalancer: public LoadBalancerSIM {  // NOLINT(whitespace/line_length)
 public:
  SC_HAS_PROCESS(LoadBalancer);
  /*Constructor*/
  explicit LoadBalancer(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~LoadBalancer() = default;

 public:
  void init();

 private:
  void LoadBalancer_PortServiceThread();
  void LoadBalancerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  void temp1();
  void temp2();

  // Port service methods
  void outgoingPackets_thread();
  void validatePacketDestination_thread();

  // Administrative methods
  void populateLocalMap();
  void mapAllServers();
  void addServerInstance_thread();
  void scheduler_thread();

  // Behavioral methods
  void establishServer(std::string clientID);

 private:
  std::map<std::string, std::string> localMap;
  MTQueue<std::shared_ptr<pfp::core::TrType> > outgoingPackets;
  std::map<std::string, struct ConnectionDetails> balancerDetails;
  std::shared_ptr<TestbedPacket> receivedPacket;
};

#endif  // BEHAVIOURAL_LOADBALANCER_H_
