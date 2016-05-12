#ifndef BEHAVIOURAL_TCPSERVER_H_
#define BEHAVIOURAL_TCPSERVER_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/TCPServerSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class TCPServer: public TCPServerSIM {
 public:
  SC_HAS_PROCESS(TCPServer);
  /*Constructor*/
  explicit TCPServer(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~TCPServer() = default;

 public:
  void init();

 private:
  void TCPServer_PortServiceThread();
  void TCPServerThread(std::size_t thread_id);
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

#endif  // BEHAVIOURAL_TCPSERVER_H_
