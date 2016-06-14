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

#include "./UDPClient.h"
#include <string>
#include <map>
#include <utility>
#include <vector>

UDPClient::UDPClient(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):UDPClientSIM(nm,parent,configfile),outlog(OUTPUTDIR+"ClientConnections.csv") {  //  NOLINT
  std::istringstream cf(configfile);
  TestbedUtilities util;
  populateLocalMap();
  ncs = util.getClientConfigurations(local_map, configfile);
  if (ncs.archive) {
    std::string full_name;
    getline(cf, full_name, '.');
    full_name.append("_client.pcap");
    pcap_logger = std::make_shared<PcapLogger>(full_name.c_str());
  }
  outlog << "LogicalTime,Client,Server" << endl;
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
  local_map.insert(std::pair<std::string, std::string>("type", tempstr));
  tempstr = GetParameter("simulationTime").get();
  local_map.insert(std::pair<std::string, std::string>
    ("simulationTime", tempstr));
  tempstr = GetParameter("virtualInstances").get();
  local_map.insert(std::pair<std::string, std::string>
    ("virtualInstances", tempstr));
  tempstr = GetParameter("archive").get();
  local_map.insert(std::pair<std::string, std::string>("archive", tempstr));
  tempstr = GetParameter("headers").get();
  local_map.insert(std::pair<std::string, std::string>("headers", tempstr));
  tempstr = GetParameter("delays").get();
  local_map.insert(std::pair<std::string, std::string>("delays", tempstr));
  tempstr = GetParameter("delayUnit").get();
  local_map.insert(std::pair<std::string, std::string>("delayUnit", tempstr));
  tempstr = GetParameter("delayDist").get();
  local_map.insert(std::pair<std::string, std::string>("delayDist", tempstr));
  tempstr = GetParameter("dhcpPolicy").get();
  local_map.insert(std::pair<std::string, std::string>("dhcpPolicy", tempstr));
  tempstr = GetParameter("dhcpPool").get();
  local_map.insert(std::pair<std::string, std::string>("dhcpPool", tempstr));
  tempstr = GetParameter("tos").get();
  local_map.insert(std::pair<std::string, std::string>("tos", tempstr));
  tempstr = GetParameter("ttl").get();
  local_map.insert(std::pair<std::string, std::string>("ttl", tempstr));
  tempstr =  GetParameter("sport").get();
  local_map.insert(std::pair<std::string, std::string>("sport", tempstr));
  tempstr = GetParameter("dport").get();
  local_map.insert(std::pair<std::string, std::string>("dport", tempstr));
  tempstr = GetParameter("se_addr").get();
  local_map.insert(std::pair<std::string, std::string>("se_addr", tempstr));
  tempstr = GetParameter("dns_load_balancer").get();
  local_map.insert(std::pair<std::string, std::string>("dns_load_balancer",
    tempstr));
}
// Administrative methods
void UDPClient::addClientInstances() {
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
    client_instances.insert(std::pair<std::string, struct ConnectionDetails>
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
    // Configuration specifies zero live instances!

    return;
  }
  if (ncs.end_time.to_seconds() == 0) {
    // Configuration specifies zero life time

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
      // Simulation done for specified time! No more
      // client instances will be activated! Waiting for exisiting processing
      // to end.
      return;
    }
    // size_t activeClients = 0;
    for (std::map<std::string, struct ConnectionDetails>::iterator it
    = client_instances.begin(); it != client_instances.end(); ++it) {
      if (!it->second.active) {
        // Activating a client Instance.
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
        // Adding client instance with idle delay of: waittime
        it->second.connection_state = serverQuery;
        it->second.idle_pending = waittime;
        it->second.file_pending = 0;
        it->second.active = true;
        // Activating: it->first
        // Initiate sending of the SYN packet
        received_packet = NULL;
        acquireServerInstance(it->first);
        // We activated a client instance. Waiting for it to connect!
        wait(activate_client_instance_event);
        // establishConnection(it->first);
      }
    }
    // Wait for a client instance to be activated
    // A client instance will be activated if *timed out*
    // or after its UDP file transfer is done
    // We activated the specified number of client instances.
    // Waiting for next activation request!
    wait(reactivate_client_instance_event);
    // reactivate_client_instance_event notified.
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
  if (client_instances.empty()) {
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
    // Configuration specifies zero live instances!

    return;
  } else if (ncs.end_time.to_seconds() == 0) {
    // Configuration specifies zero life
    return;
  }
  double rtime = 1;
  while (true) {
    if (ncs.end_time.to_seconds() != 0 && ncs.end_time <= sc_time_stamp()) {
      // Simulation done for specified time! All files end!
      return;
    }
    int idleInstances = 0;
    bool clWakeup = false;
    sc_time minTime;
    std::string minTimeCID;
    for (std::map<std::string, ConnectionDetails>::iterator it =
      client_instances.begin(); it != client_instances.end(); ++it) {
      if (it->second.connection_state == idle) {
        if (it->second.wakeup <= sc_time_stamp()) {
          it->second.active = false;
          it->second.connection_state = serverQuery;
          // Re-activating client instance for the finished file. Wakeup!
          // cout << "wake up client" << endl;
          reactivate_client_instance_event.notify();
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
      if (idleInstances == client_instances.size() &&
        !client_instances.empty()) {
         // All client instances are idle[idleInstances]!
         // Going for a wait now for minTime
         wait(minTime);
         client_instances.find(minTimeCID)->second.active = false;
         // FOR ALL OTHER client instances which are also idle state,
         // this amount should be deducted
         for (std::map<std::string, ConnectionDetails>::iterator it
           = client_instances.begin(); it != client_instances.end(); ++it) {
           if (it->second.connection_state == idle) {
             it->second.idle_pending -= minTime;
           }
         }
        // Activating client instance for the finished file.
        // npulog(profile, cout << "client idle time over" << endl;)

        reactivate_client_instance_event.notify();
        wait(SC_ZERO_TIME);
        rtime = 1;
       } else {
         // wait for resolution time
         wait(rtime, SC_NS);
         rtime = rtime*2;
         // udp client is uncontrolled!
       }
    }
  }
}
void UDPClient::outgoingPackets_thread() {
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoing_packets.pop());
    if (!out->nb_can_put()) {
      // Client stuck at MUX Ingress! This is bad!
      gotStuck = true;
    }
    out->put(packet);
    if (ncs.archive) {
      pcap_logger->logPacket(packet->setData(), sc_time_stamp());
    }
    if (gotStuck) {
      // Client resumed packet flow to ingress of MUX
      gotStuck = false;
    }
  }
}
// Behavioral methods
void UDPClient::validatePacketDestination_thread() {
  while (true) {
    received_packet = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
    struct ConnectionDetails cdet;
    if (client_instances.find(clientID) == client_instances.end()) {
      // Strange! We got a packet we have nothing to do with! Ignored
      continue;
    } else {
      cdet = client_instances.find(clientID)->second;
    }
    switch (cdet.connection_state) {
      case serverQuery:
        acquireServerInstance("0");
        break;
      case connectionSetup:
        // No connectionSetup is required for UDP connections
        // After getting server, we directly send file req
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
void UDPClient::acquireServerInstance(std::string clientID) {
  TestbedUtilities util;
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  hdrList.push_back("dns_t");
  if (received_packet == NULL) {
    std::shared_ptr<TestbedPacket> reqPacket =
      std::make_shared<TestbedPacket>();
    std::shared_ptr<TestbedPacket> resPacket =
      std::make_shared<TestbedPacket>();
    struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
    reqPacket->setData().insert(reqPacket->setData().begin(),
      cdet->received_header.begin(),
      cdet->received_header.end());
    // Client sending DNS request!"
    util.getDnsPacket(reqPacket, resPacket, 0, hdrList,
      GetParameter("se_addr").get());
    util.finalizePacket(resPacket, hdrList);
    outgoing_packets.push(resPacket);
  } else {
    std::string serverID = util.getDNSResponseIPAddr(received_packet, hdrList);
    clientID = util.getIPAddress(received_packet->getData(),
      hdrList, "dst");
    // Client received DNS response.
    npulog(profile, cout << Red << clientID << " is assigned with " << serverID
      << txtrst << endl;)
    outlog << sc_time_stamp().to_default_time_units() << ","
      << clientID << ","
      << serverID << endl;  // NOLINT
    struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
    cdet->connection_state = connectionSetup;
    received_packet = NULL;
    establishConnection(clientID, serverID);
  }
}
void UDPClient::establishConnection(std::string clientID,
  std::string serverID) {
  // When a client is instantiated it sends a fileID to the server as a
  // part of the file request
  TestbedUtilities util;
  if (received_packet == NULL) {
    // Send the fileID
    std::shared_ptr<TestbedPacket> reqPacket
    = std::make_shared<TestbedPacket>();
    struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
    reqPacket->setData().insert(reqPacket->setData().begin(),
      cdet->received_header.begin(),
      cdet->received_header.end());
    util.updateAddress(reqPacket, ncs.list, serverID, "dst");
    reqPacket->setData().push_back(129);
    // Client util.getIPAddress(reqPacket->getData(), ncs.list, "src")
    // is sending FileID to
    // util.getIPAddress(reqPacket->getData(), ncs.list, "dst")
    // as part of file request!
    util.finalizePacket(reqPacket, ncs.list);
    outgoing_packets.push(reqPacket);
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
  size_t pktsize = received_packet->setData().size();
  std::string clientID = util.getIPAddress(received_packet->getData(),
  ncs.list, "dst");
  // Checking for payload of 1 byte
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;
  int32_t fileSize = 0;
  if (payloadLen == 4) {
    activate_client_instance_event.notify();
    uint32_t *fsptr =
    static_cast<uint32_t*>(
      static_cast<void*>(received_packet->setData().data()+headerLen));
    fileSize = *fsptr;
  } else {
    // We are expecting a file size of 4 bytesfrom the server
    // Ignoring till we get one.
    return;
  }
  // Client received filesize of the requested fileID: fileSize
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = fileProcessing;
  cdet->file_pending = fileSize;
}
void UDPClient::processFile() {
  // We get files split into packets
  // If the pendingPL in the cdet structure comes down to zero we move to the
  // idle phase, where we calculate the idlePending time and then our client
  // instance goes for a sleep.
  TestbedUtilities util;
  size_t pktsize = received_packet->setData().size();
  std::string clientID =
    util.getIPAddress(received_packet->getData(), ncs.list, "dst");
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;

  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->file_pending -= payloadLen;

  // Client received file packet. Pending payload: cdet->file_pending
  if (cdet->file_pending <= 0) {
    // going to idle state and update the wake up time as well
    // Client going to idle state."
    cdet->connection_state = idle;
    cdet->wakeup = cdet->idle_pending + sc_time_stamp();
  } else {
    cdet->connection_state = fileProcessing;
  }
}
void UDPClient::teardownConnection() {
  // Unused for UDP connections
}
