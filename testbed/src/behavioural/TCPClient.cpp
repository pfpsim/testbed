/*
 * testbed: Simulation environment for PFPSim Framework models
 *
 * Copyright (C) 2016 Concordia Univ., Montreal
 *     Samar Abdi
 *     Umair Aftab
 *     Gordon Bailey
 *     Faras Dewal
 *     Shafigh Parsazad
 *     Eric Tremblay
 *
 * Copyright (C) 2016 Ericsson
 *     Bochra Boughzala
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "./TCPClient.h"
#include <string>
#include <map>
#include <utility>
#include <vector>

TCPClient::TCPClient(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):TCPClientSIM(nm,parent,configfile) {  // NOLINT
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
    sc_bind(&TCPClient::activateClientInstance_thread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&TCPClient::scheduler_thread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&TCPClient::validatePacketDestination_thread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&TCPClient::outgoingPackets_thread, this)));
}
void TCPClient::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TCPClient::TCPClient_PortServiceThread() {
  // Thread function to service input ports.
}
void TCPClient::TCPClientThread(size_t thread_id) {
  // Thread function for module functionalty
}
void TCPClient::populateLocalMap() {
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
  tempstr = GetParameter("dnspolicy").get();
  localMap.insert(std::pair<std::string, std::string>("dnspolicy", tempstr));
  tempstr = GetParameter("dnsmsq").get();
  localMap.insert(std::pair<std::string, std::string>("dnsmsq", tempstr));
  tempstr = GetParameter("tos").get();
  localMap.insert(std::pair<std::string, std::string>("tos", tempstr));
  tempstr = GetParameter("ttl").get();
  localMap.insert(std::pair<std::string, std::string>("ttl", tempstr));
  tempstr =  GetParameter("sport").get();
  localMap.insert(std::pair<std::string, std::string>("sport", tempstr));
  tempstr = GetParameter("dport").get();
  localMap.insert(std::pair<std::string, std::string>("dport", tempstr));
  tempstr = GetParameter("se_addr").get();
  localMap.insert(std::pair<std::string, std::string>("se_addr", tempstr));
  tempstr = GetParameter("dnsserver").get();
  localMap.insert(std::pair<std::string, std::string>("dnsserver", tempstr));
}
// Administrative methods
void TCPClient::addClientInstances() {
  TestbedUtilities util;
  for (std::vector<uint8_t> header : ncs.header_data) {
    std::string clientID = util.getIPAddress(header, ncs.list, "src");
    struct ConnectionDetails cdet;
    cdet.connection_state = serverQuery;
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
void TCPClient::activateClientInstance_thread() {
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
      npulog(minimal, cout << "Simulation done for specified time! No more "
      << "client instances will be activated! Waiting for exisiting processing "
      << "to end." << endl;)
      return;
    }
    // size_t activeClients = 0;
    for (std::map<std::string, struct ConnectionDetails>::iterator it
    = clientDetails.begin(); it != clientDetails.end(); ++it) {
      if (!it->second.active) {
        npulog(profile, cout << "Activating a client Instance." << endl;)
        // Either add or activate a instance from allClientDetails map  and
        // send request for TCP file transfer
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
        it->second.connection_state = serverQuery;
        it->second.idle_pending = waittime;
        it->second.file_pending = 0;
        it->second.active = true;
        npulog(profile, cout << "Activating: " << it->first << endl;)
        // Initiate sending of the SYN packet
        receivedPacket = NULL;
        requestServerInstance(it->first);
        // establishConnection(it->first);
      }
    }
    // Wait for a client instance to be activated
    // A client instance will be activated if *timed out*
    // or after its TCP file transfer is done
    npulog(profile, cout << "We activated the specified number of client "
    << "instances. Waiting for next activation request!" << endl;)
    wait(activate_client_instance_event);
    npulog(profile, cout << "activate_client_instance_event notified. "
      << endl;)
  }
}
void TCPClient::scheduler_thread() {
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
      npulog(minimal, cout << "Simulation done for specified time! "
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
      if (idleInstances == clientDetails.size() && !clientDetails.empty()) {
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
         // npulog(profile, cout << "tcp client is uncontrolled!"
         // << rtime << endl;)
       }
    }
  }
}
void TCPClient::outgoingPackets_thread() {
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
void TCPClient::validatePacketDestination_thread() {
  while (true) {
    receivedPacket = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getIPAddress(receivedPacket->getData(),
    ncs.list, "dst");
    struct ConnectionDetails cdet;
    if (clientDetails.find(clientID) == clientDetails.end()) {
      npulog(profile, cout << "Strange! We got a packet we have nothing to"
      << " do with! Destined for: " <<clientID<< "! Ignored" << endl;)
      continue;
    } else {
      cdet = clientDetails.find(clientID)->second;
    }
    switch (cdet.connection_state) {
      case serverQuery:
        requestServerInstance("0");
        break;
      case connectionSetup:
        // Passing null because once we have a receivedPacket,
        // the actual headerIndex does not matter
        establishConnection("0", "0");
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
        teardownConnection();
        break;
      case idle:
        // nothing to do actually, this is handled by a thread
        // If we get more packets here, I believe they will simply be
        // rejected at this stage :D
        break;
    }
  }
}
void TCPClient::requestServerInstance(std::string clientID) {
  TestbedUtilities util;
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  hdrList.push_back("dns_t");
  if (receivedPacket == NULL) {
    std::shared_ptr<TestbedPacket> reqPacket =
      std::make_shared<TestbedPacket>();
    std::shared_ptr<TestbedPacket> resPacket =
      std::make_shared<TestbedPacket>();
    struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
    reqPacket->setData().insert(reqPacket->setData().begin(),
      cdet->received_header.begin(),
      cdet->received_header.end());
    npulog(profile, cout << "Client sending DNS request!" << endl;)
    util.getDnsPacket(reqPacket, resPacket, 0, hdrList,
      GetParameter("se_addr").get());
    util.finalizePacket(resPacket, hdrList);
    outgoingPackets.push(resPacket);
  } else {
    std::string serverID = util.getDNSResponse(receivedPacket, hdrList);
    clientID = util.getIPAddress(receivedPacket->getData(),
      hdrList, "dst");
    npulog(profile, cout << "Client received DNS response. Client " << clientID
      << " is assigned with server " << serverID << endl;)
    struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
    cdet->connection_state = connectionSetup;
    receivedPacket = NULL;
    establishConnection(clientID, serverID);
  }
}
void TCPClient::establishConnection(std::string clientID,
  std::string serverID) {
  // If the received packet is NULL, we are initiating the file
  // Create a random server sequence number and send the SYN packet
  // Else if the received packet is not NULL, it should be ACK/SYN
  // Send ACK Packet & file request packet & change state to sendFileRequest
  TestbedUtilities util;
  if (receivedPacket == NULL) {
    // Send the SYN packet
    std::shared_ptr<TestbedPacket> reqPacket =
      std::make_shared<TestbedPacket>();
    struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
    reqPacket->setData().insert(reqPacket->setData().begin(),
        cdet->received_header.begin(),
        cdet->received_header.end());
    util.updateAddress(reqPacket, ncs.list, serverID, "dst");
    npulog(profile, cout << "Client "
      << util.getIPAddress(reqPacket->getData(), ncs.list, "src")
      << " header updated with received server address"
      << util.getIPAddress(reqPacket->getData(), ncs.list, "dst") << endl;)
    npulog(profile, cout << "Client " << clientID << " sending SYN packet to "
      << serverID << "!" << endl;)
    util.finalizePacket(reqPacket, ncs.list);
    outgoingPackets.push(reqPacket);
  } else {
    // Send ACK packet and file request packet and
    // then change state to sendFileRequest
    std::shared_ptr<TestbedPacket> reqPacket
    = std::make_shared<TestbedPacket>();
    util.getResponseHeader(receivedPacket, reqPacket, -2, ncs.list);
    util.finalizePacket(reqPacket, ncs.list);
    npulog(profile, cout << "Client sending ACK packet" << endl;)
    outgoingPackets.push(reqPacket);
    requestFile();
  }
}
void TCPClient::requestFile() {
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(receivedPacket->getData(),
    ncs.list, "dst");
  std::shared_ptr<TestbedPacket> reqPacket = std::make_shared<TestbedPacket>();
  util.getResponseHeader(receivedPacket, reqPacket, -2, ncs.list);
  reqPacket->setData().push_back(129);
  util.finalizePacket(reqPacket, ncs.list);
  npulog(profile, cout << "Client sending file request packet" << endl;)
  outgoingPackets.push(reqPacket);
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  cdet->connection_state = fileResponse;
}
void TCPClient::registerFile() {
    // We get a packet with a payload of 4 bytes
    // Store the value of the payload in the cdet structure
    // Send ACK for the received file size and change state to fileTransfer
    TestbedUtilities util;
    size_t pktsize = receivedPacket->setData().size();
    std::string clientID = util.getIPAddress(receivedPacket->getData(),
      ncs.list, "dst");
    // Checking for payload of 1 byte
    size_t headerLen = util.getHeaderLength(ncs.list);
    size_t payloadLen = pktsize - headerLen;
    size_t fileSize = 0;
    if (payloadLen == 4) {
      uint32_t *fsptr = static_cast<uint32_t*>
          (static_cast<void*>(receivedPacket->setData().data() + headerLen));
      fileSize = *fsptr;
    } else {
      // We are expecting a file size of 4 bytesfrom the server
      // Ignoring till we get one.
      return;
    }
    // sending ACK packet for the received filesize
    std::shared_ptr<TestbedPacket> resPacket =
    std::make_shared<TestbedPacket>();
    util.getResponseHeader(receivedPacket, resPacket, payloadLen, ncs.list);
    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Client sending ACK for received fileSize "
            << fileSize << endl;)
    outgoingPackets.push(resPacket);

    struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
    cdet->connection_state = fileProcessing;
    cdet->file_pending = fileSize;
}
void TCPClient::processFile() {
  // We get files split into packets
  // If the file_pending in the cdet structure comes down to zero we should
  // send the RST packet
  // after the final ACK has been sent
  // This means that for this client we won't go into the
  // connectionTeardown phase
  // connectionTeardown is normaly meant for the TCP teardown handshaing signals
  // So, after the RST packet we move to the idle phase, where we
  // calculate the idle_pending time
  // And then our client instance goes for a sleep.
  TestbedUtilities util;
  size_t pktsize = receivedPacket->setData().size();
  std::string clientID = util.getIPAddress(receivedPacket->getData(),
    ncs.list, "dst");
  int headerLen = util.getHeaderLength(ncs.list);
  int payloadLen = pktsize - headerLen;
  // sending ACK packet for the received file segement
  std::shared_ptr<TestbedPacket> reqPacket = std::make_shared<TestbedPacket>();
  util.getResponseHeader(receivedPacket, reqPacket, -2, ncs.list);
  util.finalizePacket(reqPacket, ncs.list);
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  cdet->file_pending -= payloadLen;
  npulog(profile, cout << "Client sending ACK for received fileSize."
      << " Pending payload: " << cdet->file_pending << endl;)
  outgoingPackets.push(reqPacket);
  // Simply calling the teardown Connection to send the RST packet_data_type
  if (cdet->file_pending <= 0) {
    teardownConnection();
  } else {
    cdet->connection_state = fileProcessing;
  }
}
void TCPClient::teardownConnection() {
  // We are using RST to close the connection//Normally this method is
  // used to perform the handshaking signals
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(receivedPacket->getData(),
    ncs.list, "dst");
  std::shared_ptr<TestbedPacket> reqPacket
    = std::make_shared<TestbedPacket>();
  util.getResponseHeader(receivedPacket, reqPacket, -5, ncs.list);
  util.finalizePacket(reqPacket, ncs.list);
  npulog(profile, cout << "Client sending RST to close the connection."
          << endl;)
  outgoingPackets.push(reqPacket);
  // update state to idle and update the wake up time as well
  struct ConnectionDetails *cdet = &clientDetails.find(clientID)->second;
  cdet->connection_state = idle;
  cdet->wakeup = cdet->idle_pending + sc_time_stamp();
}
