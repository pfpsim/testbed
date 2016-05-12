#ifndef BEHAVIOURAL_UDPSERVER_H_
#define BEHAVIOURAL_UDPSERVER_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/UDPServerSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class UDPServer: public UDPServerSIM {
 public:
  SC_HAS_PROCESS(UDPServer);
  /*Constructor*/
  explicit UDPServer(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~UDPServer() = default;

 public:
  void init();

 private:
  void UDPServer_PortServiceThread();
  void UDPServerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  // Administrative methods
  void populateLocalMap();
  void validatePacketSource_thread();
  void outgoingPackets_thread();
  void datarateManagement_thread();

  // Behavioral methods
  void establishConnection();
  void registerFile();
  void processFile();
  void teardownConnection();

 private:
  std::ofstream outlog;
  std::shared_ptr<TestbedPacket> receivedPacket;
  std::map<std::string, std::string> localMap;
  std::shared_ptr<PcapLogger> pcapLogger;
  ServerConfigStruct ncs;
  MTQueue<std::shared_ptr<pfp::core::TrType> > outgoingPackets;
  std::map<std::string, struct ConnectionDetails> clientDetails;
};

#endif  // BEHAVIOURAL_UDPSERVER_H_
