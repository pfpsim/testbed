#include "./UDPClient.h"
#include <string>
#include <map>
#include <utility>
#include <vector>

UDPClient::UDPClient(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):UDPClientSIM(nm,parent,configfile),outlog("PacketTraceClient.csv", ios::trunc) {  //  NOLINT
  std::istringstream cf(configfile);
  TestbedUtilities util;
  populateLocalMap();
  ncs = util.getClientConfigurations(localMap, configfile);
  if (ncs.archive) {
    std::string full_name;
    getline(cf, full_name, '.');
    full_name.append("_client.pcap");
    pcapLogger = std::make_shared<PcapLogger>(full_name.c_str());
  }
  addClientInstances();
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&UDPClient::activateClientInstance_thread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&UDPClient::scheduler_thread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&UDPClient::validatePacketDestination_thread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&UDPClient::outgoingPackets_thread, this)));
}
void UDPClient::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void UDPClient::UDPClient_PortServiceThread() {
  //  Thread function to service input ports.
}
void UDPClient::UDPClientThread(std::size_t thread_id) {
  //  Thread function for module functionalty
}
void UDPClient::populateLocalMap() {
  std::string tempstr = GetParameter("type").get();
  localMap.insert(std::pair<std::string, std::string>("type", tempstr));
  tempstr = GetParameter("simulationTime").get();
  localMap.insert(std::pair<std::string, std::string>
    ("simulationTime", tempstr));
  tempstr = GetParameter("virtualInstances").get();
  localMap.insert(std::pair<std::string, std::string>
    ("virtualInstances", tempstr));
  tempstr = GetParameter("archive").get();
  localMap.insert(std::pair<std::string, std::string>("archive", tempstr));
  tempstr = GetParameter("headers").get();
  localMap.insert(std::pair<std::string, std::string>("headers", tempstr));
  tempstr = GetParameter("delays").get();
  localMap.insert(std::pair<std::string, std::string>("delays", tempstr));
  tempstr = GetParameter("delayUnit").get();
  localMap.insert(std::pair<std::string, std::string>("delayUnit", tempstr));
  tempstr = GetParameter("delayDist").get();
  localMap.insert(std::pair<std::string, std::string>("delayDist", tempstr));
  tempstr = GetParameter("cl_dnspolicy").get();
  localMap.insert(std::pair<std::string, std::string>("cl_dnspolicy", tempstr));
  tempstr = GetParameter("se_dnspolicy").get();
  localMap.insert(std::pair<std::string, std::string>("se_dnspolicy", tempstr));
  tempstr = GetParameter("cl_dnsmsq").get();
  localMap.insert(std::pair<std::string, std::string>("cl_dnsmsq", tempstr));
  tempstr = GetParameter("se_dnsmsq").get();
  localMap.insert(std::pair<std::string, std::string>("se_dnsmsq", tempstr));
  tempstr = GetParameter("tos").get();
  localMap.insert(std::pair<std::string, std::string>("tos", tempstr));
  tempstr = GetParameter("ttl").get();
  localMap.insert(std::pair<std::string, std::string>("ttl", tempstr));
  tempstr =  GetParameter("sport").get();
  localMap.insert(std::pair<std::string, std::string>("sport", tempstr));
  tempstr = GetParameter("dport").get();
  localMap.insert(std::pair<std::string, std::string>("dport", tempstr));
}
// Administrative methods
void UDPClient::addClientInstances() {
  TestbedUtilities util;
  for (std::vector<uint8_t> header : ncs.header_data) {
    std::string clientID = util.getConnectionID(header, ncs.list, "src");
    struct ConnectionDetails cdet;
    cdet.connection_state = connectionSetup;
    cdet.file_pending = 0;
    cdet.received_header.insert(cdet.received_header.begin(), header.begin(),
    header.end());
    cdet.active = false;
    cdet.fileIndex = -1;
    cdet.delayIndex = -1;
    clientDetails.insert(std::pair<std::string, struct ConnectionDetails>
      (clientID, cdet));
  }
}
void UDPClient::activateClientInstance_thread() {
  std::string maxinst = GetParameter("virtualInstances").get();
  int maxInstances = 0;
  try {
     maxInstances = stoi(maxinst);
  }catch (std::exception &e) {
    std::cerr << "Exception while parsing client/server nodes!" << endl;
    assert(false);
  }
  if (maxInstances == 0) {
    npulog(profile, cout << "Configuration specifies zero live instances!"
    << endl;)
    return;
  }
  if (ncs.end_time.to_seconds() == 0) {
    npulog(profile, cout << "Configuration specifies zero life time"
    << endl;)
    return;
  }
  size_t maxHeaderIndex = ncs.header_data.size();
  size_t delayLen = ncs.delay.delay_values.size();
  if (maxInstances > maxHeaderIndex) {
    std::cerr << "FATAL ERROR: Check the configuration file. Maximum instances"
    << " cannot be more than number of headers" << endl;
    assert(false);
  }
  TestbedUtilities util;
  while (true) {
    // Should we continue the packet generation?
    if (ncs.end_time.to_seconds() != 0 && ncs.end_time <= sc_time_stamp()) {
      npulog(profile, cout << "Simulation done for specified time! No more "
      << "client instances will be activated! Waiting for exisiting processing "
      << "to end." << endl;)
      return;
    }
    size_t activeClients = 0;
    for (std::map<std::string, struct ConnectionDetails>::iterator it
    = clientDetails.begin(); it != clientDetails.end(); ++it) {
      if (!it->second.active) {
        npulog(profile, cout << "Activating a client Instance." << endl;)
        // Either add or activate a instance from allClientDetails map  and
        // send request for UDP file transfer
        if (ncs.delay.distribution.type.compare("round-robin") == 0) {
         it->second.delayIndex++;
         if (it->second.delayIndex == delayLen) {
           it->second.delayIndex = 0;
         }
        } else {
          it->second.delayIndex = util.getRandomNum(0, delayLen - 1,
            ncs.delay.distribution.type, ncs.delay.distribution.param1,
            ncs.delay.distribution.param2);
        }
        sc_time waittime = ncs.delay.delay_values.at(it->second.delayIndex);
        npulog(profile, cout << "Adding client instance with idle delay of: "
        << waittime << endl;)
        it->second.connection_state = connectionSetup;
        it->second.idle_pending = waittime;
        it->second.file_pending = 0;
        it->second.active = true;
        npulog(profile, cout << "Activating: " << it->first << endl;)
        // Initiate sending of the SYN packet
        receivedPacket = NULL;
        establishConnection(it->first);
      }
    }
    // Wait for a client instance to be activated
    // A client instance will be activated if *timed out*
    // or after its UDP file transfer is done
    npulog(profile, cout << "We activated the specified number of client "
    << "instances. Waiting for next activation request!" << endl;)
    wait(activate_client_instance_event);
    npulog(profile, cout << "activate_client_instance_event notified. "
      << endl;)
  }
}
void UDPClient::scheduler_thread() {
  // If all instances have idle state, we should look for the one with the
  // minimum wake time and sleep for that much amount of time
  // Else, we continue our execution of the client instances
  // In between our runs of this infinite loop, we should also wait for a
  // resolution time delay.
  // If the current time reaches the simulation end time, or if the number
  // of received files will equal the configured file transfers, the instances
  // will be stopped from spawnning by the addNewClientInstance
  if (clientDetails.empty()) {
    // This thread cannot be executed like this :)
    wait(SC_ZERO_TIME);
  }
  std::string maxinst = GetParameter("virtualInstances").get();
  int maxInstances = 0;
  try {
     maxInstances = stoi(maxinst);
  }catch (std::exception &e) {
    std::cerr << "Exception while parsing client/server nodes!" << endl;
    assert(false);
  }
  if (maxInstances == 0) {
    npulog(profile, cout << "Configuration specifies zero live instances!"
    << endl;)
    return;
  } else if (ncs.end_time.to_seconds() == 0) {
    npulog(profile, cout << "Configuration specifies zero life" << endl;)
    return;
  }
  double rtime = 1;
  while (true) {
    if (ncs.end_time.to_seconds() != 0 && ncs.end_time <= sc_time_stamp()) {
      npulog(profile, cout << "Simulation done for specified time! "
      << "All files end!" << endl;)
      return;
    }
    int idleInstances = 0;
    bool clWakeup = false;
    sc_time minTime;
    std::string minTimeCID;
    for (std::map<std::string, ConnectionDetails>::iterator it =
      clientDetails.begin(); it != clientDetails.end(); ++it) {
      if (it->second.connection_state == idle) {
        if (it->second.wakeup <= sc_time_stamp()) {
          it->second.active = false;
          npulog(profile, cout << "Re-activating client instance for the "
          << "finished file. Wakeup!" << endl;)
          activate_client_instance_event.notify();
          // Allow the client thread to be instantiated immediately by yielding
          wait(SC_ZERO_TIME);
          clWakeup = true;
          rtime = 1;
          break;
        }
        if (idleInstances == 0) {
          minTime = it->second.idle_pending;
          minTimeCID = it->first;
        }
        idleInstances++;
        if (it->second.idle_pending < minTime) {
          minTime = it->second.idle_pending;
          minTimeCID = it->first;
        }
      } else {
        break;
      }
    }
    if (!clWakeup) {
      if (idleInstances == clientDetails.size() && clientDetails.size() != 0) {
         npulog(profile, cout << "All client instances are idle["
         << idleInstances << "]! Going for a wait now for "
         << minTime << " ! " << endl;)
         wait(minTime);
         clientDetails.find(minTimeCID)->second.active = false;
         // FOR ALL OTHER client instances which are also idle state,
         // this amount should be deducted
         for (std::map<std::string, ConnectionDetails>::iterator it
           = clientDetails.begin(); it != clientDetails.end(); ++it) {
           if (it->second.connection_state == idle) {
             it->second.idle_pending -= minTime;
           }
         }
        npulog(profile, cout << "Activating client instance for the finished"
        << " file." << endl;)
        activate_client_instance_event.notify();
        wait(SC_ZERO_TIME);
        rtime = 1;
       } else {
         // wait for resolution time
         wait(rtime, SC_NS);
         rtime = rtime*2;
         npulog(profile, cout << "udp client is uncontrolled!"
         << rtime << endl;)
       }
    }
  }
}
void UDPClient::validatePacketDestination_thread() {
  while (true) {
    receivedPacket = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getConnectionID(receivedPacket->getData(),
    ncs.list);
    struct ConnectionDetails cdet;
    if (clientDetails.find(clientID) == clientDetails.end()) {
      npulog(profile, cout << "Strange! We got a packet we have nothing to"
      << " do with! Destined for: " <<clientID<< "! Ignored" << endl;)
    } else {
      cdet = clientDetails.find(clientID)->second;
    }
    switch (cdet.connection_state) {
      case connectionSetup:
        // No connectionSetup is required for UDP connections
        // sendFileID(0);
        break;
      case fileRequest:
        requestFile();
        break;
      case fileResponse:
        registerFile();
        break;
      case fileProcessing:
        processFile();
        break;
      case connectionTeardown:
        // No connectionTeardown in required for UDP connections
        break;
      case idle:
        // nothing to do actually, this is handled by a thread
        // If we get more packets here, I believe they will simply be
        // rejected at this stage :D
        break;
    }
  }
}
void UDPClient::outgoingPackets_thread() {
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoingPackets.pop());
    if (!out->nb_can_put()) {
      npulog(profile, cout << "Client stuck at MUX Ingress!"
          << " This is bad! Logical Time: " << sc_time_stamp() << endl;)
      gotStuck = true;
    }
    out->put(packet);
    if (ncs.archive) {
      pcapLogger->logPacket(packet->setData(), sc_time_stamp());
    }
    if (gotStuck) {
      npulog(profile, cout << "Client resumed packet flow to ingress of MUX"
        << "at logical time: " << sc_time_stamp() << endl;)
      gotStuck = false;
    }
  }
}
// Behavioral methods
void UDPClient::establishConnection(std::string clientID) {
  // When a client is instantiated it sends a fileID to the server as a
  // part of the file request
  TestbedUtilities util;
  if (receivedPacket == NULL) {
    // Send the fileID
    std::shared_ptr<TestbedPacket> reqPacket
    = std::make_shared<TestbedPacket>();
    struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
    reqPacket->setData().insert(reqPacket->setData().begin(),
      cdet->received_header.begin(),
      cdet->received_header.end());
    reqPacket->setData().push_back(129);
    npulog(profile, cout << "Client sending FileID as part of file request!"
    << endl;)
    util.finalizePacket(reqPacket, ncs.list);
    outgoingPackets.push(reqPacket);
    cdet->connection_state = fileResponse;
  }
}
void UDPClient::requestFile() {
  // The connection establishment itself is actually requestTransaction
}
void UDPClient::registerFile() {
  // We get a packet with a payload of 4 bytes
  // Store the value of the payload in the cdet structure
  // Change state to fileTransfer
  TestbedUtilities util;
  size_t pktsize = receivedPacket->setData().size();
  std::string clientID = util.getConnectionID(receivedPacket->getData(),
  ncs.list);
  // Checking for payload of 1 byte
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;
  int32_t fileSize = 0;
  if (payloadLen == 4) {
    uint32_t *fsptr =
    static_cast<uint32_t*>(
      static_cast<void*>(receivedPacket->setData().data()+headerLen));
    fileSize = *fsptr;
  } else {
    // We are expecting a file size of 4 bytesfrom the server
    // Ignoring till we get one.
    return;
  }
  npulog(profile, cout << "Client received filesize of the requested fileID:"
  << fileSize << endl;)
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  cdet->connection_state = fileProcessing;
  cdet->file_pending = fileSize;
}
void UDPClient::processFile() {
  // We get files split into packets
  // If the pendingPL in the cdet structure comes down to zero we move to the
  // idle phase, where we calculate the idlePending time and then our client
  // instance goes for a sleep.
  TestbedUtilities util;
  size_t pktsize = receivedPacket->setData().size();
  std::string clientID =
    util.getConnectionID(receivedPacket->getData(), ncs.list);
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;

  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  cdet->file_pending -= payloadLen;

  npulog(profile, cout << "Client received file packet. Pending payload: "
  << cdet->file_pending << endl;)
  if (cdet->file_pending <= 0) {
    // going to idle state and update the wake up time as well
    npulog(profile, cout << "Client going to idle state." << endl;)
    cdet->connection_state = idle;
    cdet->wakeup = cdet->idle_pending + sc_time_stamp();
  } else {
    cdet->connection_state = fileProcessing;
  }
}
void UDPClient::teardownConnection() {
  // Unused for UDP connections
}