#include "./TCPServer.h"
#include <string>
#include <utility>
#include <map>
#include <vector>

TCPServer::TCPServer(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):TCPServerSIM(nm,parent,configfile),outlog("PacketTraceServer.csv", ios::trunc) {  //  NOLINT
  std::istringstream cf(configfile);
  TestbedUtilities util;
  populateLocalMap();
  ncs = util.getServerConfigurations(localMap, configfile);
  if (ncs.archive) {
    std::string full_name;
    std::getline(cf, full_name, '.');
    full_name.append("_server.pcap");
    pcapLogger = std::make_shared<PcapLogger>(full_name.c_str());
  }
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TCPServer::validatePacketSource_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TCPServer::datarateManagement_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TCPServer::outgoingPackets_thread, this)));
}

void TCPServer::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TCPServer::TCPServer_PortServiceThread() {
  //  Thread function to service input ports.
}
void TCPServer::TCPServerThread(std::size_t thread_id) {
  //  Thread function for module functionalty
}
void TCPServer::populateLocalMap() {
  std::string tempstr = GetParameter("type").get();
  localMap.insert(std::pair<std::string, std::string>("type", tempstr));
  tempstr = GetParameter("mtu").get();
  localMap.insert(std::pair<std::string, std::string>("mtu", tempstr));
  tempstr = GetParameter("archive").get();
  localMap.insert(std::pair<std::string, std::string>("archive", tempstr));
  tempstr = GetParameter("headers").get();
  localMap.insert(std::pair<std::string, std::string>("headers", tempstr));
  tempstr = GetParameter("sizes").get();
  localMap.insert(std::pair<std::string, std::string>("sizes", tempstr));
  tempstr = GetParameter("sizeUnit").get();
  localMap.insert(std::pair<std::string, std::string>("sizeUnit", tempstr));
  tempstr = GetParameter("sizeDist").get();
  localMap.insert(std::pair<std::string, std::string>("sizeDist", tempstr));
  tempstr = GetParameter("datarate").get();
  localMap.insert(std::pair<std::string, std::string>("datarate", tempstr));
}

// Administrative methods
void TCPServer::validatePacketSource_thread() {
  while (true) {
    // If we don't get any packet for an hour,
    // we would infer that the clients are all
    // terminated. And hence, we terminate the server also.
    sc_time maxINwait = sc_time(3600, SC_SEC);
    if (!in->nb_can_get()) {
      sc_time stTime = sc_time_stamp();
      npulog(profile, cout << "tcp server going for wait @ " << stTime
      << endl;)
      wait(maxINwait, in->ok_to_get());
      sc_time endTime = sc_time_stamp();
      npulog(profile, cout << "tcp server came out of wait @ " << endTime
      << endl;)
      if (endTime - stTime >= maxINwait) {
        for (sc_process_handle temp : ThreadHandles) {
          npulog(profile, cout << "tcp server killing all its threads"
          << endl;)
          temp.kill();
        }
      }
    }
    receivedPacket = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getConnectionID(receivedPacket->getData(),
      ncs.list);
    // Check if we have an ongoing file with the client
    // If yes, call the method to execute sending the payload
    // else, execute the method to establish TCP connection
    struct ConnectionDetails cdet;
    if (clientDetails.find(clientID) == clientDetails.end()) {
      npulog(profile, cout << "Server got new request from client: "
      << clientID << endl;)
      cdet.connection_state = connectionSetup;
      cdet.fileIndex = -1;
      cdet.file_pending = 0;
      cdet.active = true;
      clientDetails.insert(
        std::pair<std::string, struct ConnectionDetails>(clientID, cdet));
    } else {
      cdet = clientDetails.find(clientID)->second;
      if (cdet.active == false) {
        npulog(profile, cout << "Reactivating an old connection" << endl;)
        cdet.connection_state = connectionSetup;
        cdet.file_pending = 0;
        cdet.active = true;
        clientDetails[clientID] = cdet;
        npulog(profile, cout << "Prev file index was: " << cdet.fileIndex
        << endl;)
      } else {
        npulog(profile, cout << "Continuation of an active connection"
          << endl;)
      }
    }
    switch (cdet.connection_state) {
      case connectionSetup:
        establishConnection();
        break;
      case fileRequest:
        // Server's do not send requests for files
        break;
      case fileResponse:
        registerFile();
        break;
      case fileProcessing:
        processFile();
        break;
      case connectionTeardown:
        teardownConnection();
        break;
      case idle:
        // TCP Servers don't have timing.. they get blocked on incoming packets
        // UDP Servers use this stage for datarate management :)
        break;
    }
  }
}
void TCPServer::outgoingPackets_thread() {
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoingPackets.pop());
    if (!out->nb_can_put()) {
      npulog(profile, cout << "Server stuck at MUX Ingress! This is bad!"
      << endl;)
      gotStuck = true;
    }
    out->put(packet);
    if (ncs.archive) {
      pcapLogger->logPacket(packet->setData(), sc_time_stamp());
    }
    if (gotStuck) {
      npulog(profile, cout << "Server resumed packet flow to ingress of MUX"
      << endl;)
      gotStuck = false;
    }
  }
}
void TCPServer::datarateManagement_thread() {
  // Not needed for TCP Servers for now
}

// Behavioral methods
void TCPServer::establishConnection() {
  // The received packet must have the SYN flag set
  // Create a random server sequence number and send the ACK/SYN packet
  // Change state of server for this source to sendFileSize
  TestbedUtilities util;
  std::string clientID = util.getConnectionID(receivedPacket->getData(),
    ncs.list);
  std::vector<uint8_t> tcpvec = util.getLayer4Header(
    receivedPacket->getData(), ncs.list);
  struct tcphdr* tcpheader = (struct tcphdr*)(tcpvec.data());
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  if (tcpheader->th_flags == TH_SYN) {
    std::shared_ptr<TestbedPacket> resPacket
    = std::make_shared<TestbedPacket>();
    util.getResponseHeader(receivedPacket, resPacket, -1, ncs.list);
    npulog(profile, cout << "ReceivedPacket size: "
    << receivedPacket->setData().size() << "! Response packet size: "
    << resPacket->setData().size() << endl;)

    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Server sending packet. ACK/SYN response. Size: "
    << resPacket->setData().size() << endl;)
    outgoingPackets.push(resPacket);
    cdet->connection_state = fileResponse;
    // clientDetails[srcIP] = cdet;
  }
}
void TCPServer::registerFile() {
  // Keep receiving packets at this state unless we get a fileID request
  // A fileID request is a packet with a payload of size 1 byte
  // We should have a corresponding element in our filesizes vector
  // Once we receive a valid request, update the pendingFileSize vector
  // Then change the state to sendFile
  TestbedUtilities util;
  size_t pktsize = receivedPacket->setData().size();
  std::string clientID = util.getConnectionID(receivedPacket->getData(),
    ncs.list);
  // Checking for payload of 1 byte
  int headerLen = util.getHeaderLength(ncs.list);
  int payloadLen = pktsize - headerLen;
  if (payloadLen == 1) {
    int fileID = 0;
    fileID = static_cast<int>(receivedPacket->setData().back());
    if (fileID == 129) {
      npulog(cout << "yuhuu";)
    } else {
      npulog(cout << "boohoo";)
    }
  } else {
    // We are expecting a file request from this client.
    // Ignoring till we get one.
    return;
  }
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;

  int fileLen = ncs.fsize.size_values.size();
  if (ncs.fsize.distribution.type.compare("round-robin") == 0) {
    cdet->fileIndex++;
    if (cdet->fileIndex == fileLen) {
      cdet->fileIndex = 0;
    }
  } else {
    cdet->fileIndex = util.getRandomNum(0, fileLen - 1,
      ncs.fsize.distribution.type, ncs.fsize.distribution.param1,
      ncs.fsize.distribution.param2);
  }

  // We return the file corresponding to the provided fileID
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();
  util.getResponseHeader(receivedPacket, resPacket, payloadLen, ncs.list);


  // We can multiply the video encoding rate with the duration to get the
  // total video duration
  int32_t fileSize = ncs.fsize.size_values.at(cdet->fileIndex);

  uint32_t fsTemp = (uint32_t)fileSize;
  uint8_t *fsptr = static_cast<uint8_t*>(static_cast<void*>(&fsTemp));
  resPacket->setData().insert(resPacket->setData().end(), fsptr, fsptr+4);

  util.finalizePacket(resPacket, ncs.list);
  npulog(profile, cout << "Server sending packet. File Size: " << fileSize
  << endl;)
  outgoingPackets.push(resPacket);

  cdet->connection_state = fileProcessing;
  cdet->file_pending = fileSize;
  // clientDetails[srcIP] = cdet;
}
void TCPServer::processFile() {
  // Send the next packet untill the pending file size becomes zero
  // Then change the state to connectionTeardown
  TestbedUtilities util;
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();

  std::string clientID = util.getConnectionID(receivedPacket->getData(),
    ncs.list);
  int maxPayload = ncs.mtu - util.getHeaderLength(ncs.list)
    + sizeof(ether_header);
  std::vector<uint8_t> tcpvec = util.getLayer4Header(
    receivedPacket->getData(), ncs.list);
  struct tcphdr* tcpheader = (struct tcphdr*)(tcpvec.data());
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  int actPayload = (cdet->file_pending > maxPayload)
    ? maxPayload:cdet->file_pending;
  if (cdet->file_pending == 0 && tcpheader->th_flags == TH_ACK) {
      npulog(profile, cout << "Server received ACK for final file packet. "
      << "Changing serverstate to connectionTearDown" << endl;)
      cdet->connection_state = connectionTeardown;
  } else {
    // Expecting only ACK packets at this stage
    int headerLen = util.getHeaderLength(ncs.list);
    int pktsize = receivedPacket->setData().size();
    int payloadLen = pktsize - headerLen;
    if (payloadLen != 0) {
      npulog(profile, cout << "Received a non zero payload during file "
      << "transmission!" << endl;)
      return;
      // we are assuming will be the ack for the previously send packet.
    }
    util.getResponseHeader(receivedPacket, resPacket, payloadLen, ncs.list);
    util.addPayload(resPacket, actPayload);

    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Server sending packet. Packet size: "
    << resPacket->getData().size() << "! File left: "
    << cdet->file_pending - actPayload << endl;)
    outgoingPackets.push(resPacket);
  }
  cdet->file_pending -= actPayload;
  // clientDetails[srcIP] = cdet;
}
void TCPServer::teardownConnection() {
  // Send the FIN/ACK request
  // Once you get the ACK for the FIN, clear up the source connection details
  TestbedUtilities util;
  std::string clientID = util.getConnectionID(receivedPacket->getData(),
    ncs.list);
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  std::vector<uint8_t> tcpvec = util.getLayer4Header(
    receivedPacket->getData(), ncs.list);
  struct tcphdr* tcpheader = (struct tcphdr*)(tcpvec.data());
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();
  if (tcpheader->th_flags == (TH_FIN | TH_ACK)
    || tcpheader->th_flags == TH_FIN) {
    util.getResponseHeader(receivedPacket, resPacket, -3, ncs.list);
    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Server sending packet. ACK/FIN " << endl;)
    outgoingPackets.push(resPacket);
  } else if (tcpheader->th_flags == TH_ACK) {
    npulog(profile, cout << "Server got teardown ACK" << endl;)
    // clientDetails.erase(srcIP);
    cdet->active = false;
  } else if (tcpheader->th_flags == TH_RST
    || tcpheader->th_flags == (TH_RST | TH_ACK)) {
    npulog(profile, cout << "Server received RST packet. "
    << "Closing connection" << endl;)
    // clientDetails.erase(srcIP);
    cdet->active = false;
  }
}
