  #ifndef BEHAVIOURAL_UDPCLIENT_H_
#define BEHAVIOURAL_UDPCLIENT_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/UDPClientSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class UDPClient: public UDPClientSIM {
 public:
  SC_HAS_PROCESS(UDPClient);
  /*Constructor*/
  explicit UDPClient(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~UDPClient() = default;

 public:
  void init();

 private:
  void UDPClient_PortServiceThread();
  void UDPClientThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  // Port service methods
  void outgoingPackets_thread();
  void validatePacketDestination_thread();

  // Administrative methods
  void populateLocalMap();
  void addClientInstances();
  void activateClientInstance_thread();
  void scheduler_thread();

  // Behavioral methods
  void establishConnection(std::string clientID);
  void requestFile();
  void registerFile();
  void processFile();
  void teardownConnection();


 private:
  std::ofstream outlog;
  sc_event activate_client_instance_event;
  std::map<std::string, std::string> localMap;
  std::shared_ptr<PcapLogger> pcapLogger;
  ClientConfigStruct ncs;
  MTQueue<std::shared_ptr<pfp::core::TrType> > outgoingPackets;
  std::map<std::string, struct ConnectionDetails> clientDetails;
  std::shared_ptr<TestbedPacket> receivedPacket;
};

#endif  // BEHAVIOURAL_UDPCLIENT_H_
