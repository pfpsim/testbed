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

#include "./UDPServer.h"
#include <string>
#include <utility>
#include <map>
#include <vector>

UDPServer::UDPServer(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):UDPServerSIM(nm,parent,configfile) {  //  NOLINT
  std::istringstream cf(configfile);
  TestbedUtilities util;
  populateLocalMap();
  ncs = util.getServerConfigurations(local_map, configfile);
  if (ncs.archive) {
    std::string full_name;
    std::getline(cf, full_name, '.');
    full_name.append("_server.pcap");
    pcap_logger = std::make_shared<PcapLogger>(full_name.c_str());
  }
  initializeServer();
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&UDPServer::outgoingPackets_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&UDPServer::datarateManager_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&UDPServer::validatePacketSource_thread, this)));
  // serverSessionsManager();
}

void UDPServer::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void UDPServer::UDPServer_PortServiceThread() {
  // Thread function to service input ports.
}
void UDPServer::UDPServerThread(std::size_t thread_id) {
  // Thread function for module functionalty
}
void UDPServer::initializeServer() {
  TestbedUtilities util;
  // Creating a new session"
  std::string serverID =
    util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
  // Add server instance: serverID
  server_sessions.insert(std::pair<std::string, size_t>(serverID, 0));

  std::shared_ptr<TestbedPacket> lb_packet =
    std::make_shared<TestbedPacket>();
  util.getLoadBalancerPacket(lb_packet, server_sessions,
    GetParameter("URL").get(), ncs.prefixes.prefix_values,
    GetParameter("dns_load_balancer").get(),
    received_packet, ncs.list);
  // SimulationParameters["dns_load_balancer"].get()
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  util.finalizePacket(lb_packet, hdrList);
  // Pushing load balancing packet for table update - 01"

  outgoing_packets.push(lb_packet);
}
void UDPServer::populateLocalMap() {
  std::string tempstr = GetParameter("type").get();
  local_map.insert(std::pair<std::string, std::string>("type", tempstr));
  tempstr = GetParameter("mtu").get();
  local_map.insert(std::pair<std::string, std::string>("mtu", tempstr));
  tempstr = GetParameter("archive").get();
  local_map.insert(std::pair<std::string, std::string>("archive", tempstr));
  tempstr = GetParameter("headers").get();
  local_map.insert(std::pair<std::string, std::string>("headers", tempstr));
  tempstr = GetParameter("sizes").get();
  local_map.insert(std::pair<std::string, std::string>("sizes", tempstr));
  tempstr = GetParameter("sizeUnit").get();
  local_map.insert(std::pair<std::string, std::string>("sizeUnit", tempstr));
  tempstr = GetParameter("sizeDist").get();
  local_map.insert(std::pair<std::string, std::string>("sizeDist", tempstr));
  tempstr = GetParameter("datarate").get();
  local_map.insert(std::pair<std::string, std::string>("datarate", tempstr));
  tempstr = GetParameter("dhcpPolicy").get();
  local_map.insert(std::pair<std::string, std::string>("dhcpPolicy", tempstr));
  tempstr = GetParameter("dhcpPool").get();
  local_map.insert(std::pair<std::string, std::string>("dhcpPool", tempstr));
  tempstr = GetParameter("URL").get();
  local_map.insert(std::pair<std::string, std::string>("URL", tempstr));
}

// Administrative methods
void UDPServer::validatePacketSource_thread() {
  while (true) {
    // If we don't get any packet for an hour,
    // we would infer that the clients are all
    // terminated. And hence, we terminate the server also.
    sc_time maxINwait = sc_time(3600, SC_SEC);
    if (!in->nb_can_get()) {
      sc_time stTime = sc_time_stamp();
      // udp server going for wait @ stTime
      wait(maxINwait, in->ok_to_get());
      sc_time endTime = sc_time_stamp();
      // udp server came out of wait @ endTime
      if (endTime - stTime >= maxINwait) {
        for (sc_process_handle temp : ThreadHandles) {
          // udp server killing all its threads"
          temp.kill();
        }
      }
    }
    received_packet = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getIPAddress(received_packet->getData(),
      ncs.list, "src");
    // Check if we have an ongoing file with the client
    // If yes, call the method to execute sending the payload
    // else, execute the method to establish UDP connection
    struct ConnectionDetails cdet;
    std::string dnsreply;
    if (client_instances.find(clientID) == client_instances.end()) {
      // Server got new request from client: clientID
      // An UDP server goes to file size send mode upon receiving a
      // packet from a new client
      // Server got new session request from client: clientID
      cdet.connection_state = connectionSetup;  // serverQuery;
      cdet.fileIndex = -1;
      cdet.file_pending = 0;
      cdet.active = true;
      client_instances.insert(
        std::pair<std::string, struct ConnectionDetails>(clientID, cdet));
      dnsreply = serverSessionsManager();
      } else {
      cdet = client_instances.find(clientID)->second;
      if (cdet.active == false) {
        // Reactivating an old connection"
        cdet.connection_state = connectionSetup;  // serverQuery;
        cdet.file_pending = 0;
        cdet.active = true;
        client_instances[clientID] = cdet;
        // Prev file index was: cdet.fileIndex
        dnsreply = serverSessionsManager();
      } else {
        // Continuation of an active connection"
      }
    }
    switch (cdet.connection_state) {
      case serverQuery:
        assignServer(dnsreply);
        break;
      case connectionSetup:
        establishConnection();
        break;
      case fileRequest:
        // a server does not request files
        break;
      case fileResponse:
        registerFile();
        break;
      case fileProcessing:
        processFile();
        break;
      case connectionTeardown:
        // tearConnection();
        break;
      case idle:
        // Used to introduce temporary delay in sending packets within a file to
        // maintain specified data rate
        // datarateManager_thread();
        break;
    }
  }
}
void UDPServer::outgoingPackets_thread() {
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoing_packets.pop());
    if (!out->nb_can_put()) {
      // Server stuck at MUX Ingress! This is bad!
      gotStuck = true;
    }
    out->put(packet);
    if (ncs.archive) {
      pcap_logger->logPacket(packet->setData(), sc_time_stamp());
    }
    if (gotStuck) {
      // Server resumed packet flow to ingress of MUX"
      gotStuck = false;
    }
  }
}
void UDPServer::datarateManager_thread() {
  // If all instances have idle state, we should look for the one
  // with the minimum wake time and sleep for that much amount of time
  // Else, we continue our execution of the client instances
  // In between our runs of this infinite loop, we should also wait
  // for a resolution time delay.
  // If the current time reaches the simulation end time, or if the number of
  // received files will equal the configured file transfers, the instances
  // will be stopped from spawnning by the addNewClientInstance
  while (true) {
    int idleInstances = 0;
    bool clWakeup = false;
    sc_time minTime;
    std::string minTimeCID;
    for (std::map<std::string, ConnectionDetails>::iterator it
      = client_instances.begin(); it != client_instances.end(); ++it) {
      struct ConnectionDetails *cdet = &it->second;
      if (cdet->connection_state == idle) {
        if (cdet->wakeup <= sc_time_stamp()) {
          // Waking up connection to send next packet!"
          received_packet.reset();
          received_packet = std::make_shared<TestbedPacket>();
          received_packet->setData().insert(received_packet->setData().begin(),
          cdet->received_header.begin(), cdet->received_header.end());
          clWakeup = true;
          cdet->connection_state = fileProcessing;
          processFile();
          break;
        }
        if (idleInstances == 0) {
          minTime = cdet->idle_pending;
          minTimeCID = it->first;
        }
        idleInstances++;
        if (cdet->idle_pending < minTime) {
          minTime = cdet->idle_pending;
          minTimeCID = it->first;
        }
      }
    }
    if (!clWakeup) {
      if (!client_instances.empty() &&
        idleInstances == client_instances.size()) {
        // All server instances are idle[idleInstances]!
        // Going for a wait now for minTime!
        wait(minTime);
        // FOR ALL OTHER client instances which are also idle state,
        // this amount should be deducted
        for (std::map<std::string, ConnectionDetails>::iterator it =
          client_instances.begin(); it != client_instances.end(); ++it) {
          struct ConnectionDetails *cdet = &it->second;
          if (cdet->connection_state == idle) {
            cdet->idle_pending -= minTime;
          }
        }
        // Sending next packet for this connection with the min idle time
        struct ConnectionDetails *cdet =
        &client_instances.find(minTimeCID)->second;
        received_packet.reset();
        received_packet = std::make_shared<TestbedPacket>();
        received_packet->setData().insert(received_packet->setData().begin(),
        cdet->received_header.begin(), cdet->received_header.end());
        cdet->connection_state = fileProcessing;
        processFile();
      } else {
        wait(1, SC_MS);
        // Adjust the wait time within the UPD server if slow
      }
    }
  }
}
std::string UDPServer::serverSessionsManager() {
  TestbedUtilities util;
  int maxSessions = 0;
  std::string  serverID;
  try {
    std::string temp = GetParameter("sessions").get();
    maxSessions = stoi(temp);
  } catch (std::exception &exp) {
    assert(!"Server config file error. Invalid sessions");
  }
  serverID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  // ReceivedPacket serverID: serverID
  std::vector<std::string> baseIPs =
    util.getBaseIPs(ncs.prefixes.prefix_values);
  if (std::find(baseIPs.begin(), baseIPs.end(), serverID) == baseIPs.end()) {
    // The serverID does not belong to any of the base IPs
    if (server_sessions[serverID] == maxSessions - 1) {
      // WARNING: The server serverID instance is reaching its capacity -
      // server_sessions[serverID] active sessions! Creating a new session
      std::string newServerID =
        util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
      // Add server instance: newServerID
      server_sessions.insert(std::pair<std::string, size_t>(newServerID, 0));
    }
  } else {
    // Packet was received by the virtual server"

    // assign it to a new serverSession
    // The received packet is destined for the virtual server
    // 1. Check if we have any existing server_sessions which can absorb this
    // 2. If not, create a new server session and assign this request
    //    to the newly created server session
    bool addressUpdated = false;
    for (std::map<std::string, size_t>::iterator it = server_sessions.begin();
      it != server_sessions.end(); ++it) {
      if (it->second < maxSessions && !addressUpdated) {
        serverID = it->first;
        addressUpdated = true;
      }
    }
    if (!addressUpdated) {
      // add the new instance and update the address
      // Creating a new session"
      serverID =
        util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
      // Add server instance: serverID
      server_sessions.insert(std::pair<std::string, size_t>(serverID, 0));
    }
  }
  // If the current state of the received packet is serverQuery,
  // we will increase the server load
  // If the current state of the received packet is connectionTeardown,
  // we will decrease the server load
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  // Number of client instances client_instances.size()
  if (client_instances.find(clientID) == client_instances.end()) {
    // The client instance was not found??"
    assert(false);
  }
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  // Connection state is: cdet->connection_state

  if (cdet->connection_state == connectionSetup ||
    cdet->connection_state == fileResponse ||
    cdet->connection_state == serverQuery) {  // serverQuery) {
    // incrementing server_sessions count"
    server_sessions[serverID]++;
  } else if (cdet->connection_state == connectionTeardown) {
    // decrementing server_sessions count"
    server_sessions[serverID]--;
  }
  if (!server_sessions.empty()) {
    // We delete the server instances with zero load
    // At max, we keep only one server instance with zero load ;)
    size_t zero_session_servers = 0;
    std::vector<std::string> deleteInstances;
    for (std::map<std::string, size_t>::iterator it = server_sessions.begin();
      it != server_sessions.end(); ++it) {
      if (it->second <= 0) {
        zero_session_servers++;
        if (zero_session_servers > 1) {
          deleteInstances.push_back(it->first);
        }
      }
    }
    for (std::string sid : deleteInstances) {
      // delete instances with no load
      // Erase server instance: sid
      server_sessions.erase(sid);
      }
  }

  // Creating the load balancer packet after all the updates
  // we use the 0th base IP as our node ID
  std::shared_ptr<TestbedPacket> lb_packet =
    std::make_shared<TestbedPacket>();
  // Number of server sessions: server_sessions.size()
  util.getLoadBalancerPacket(lb_packet, server_sessions,
    GetParameter("URL").get(), ncs.prefixes.prefix_values,
    GetParameter("dns_load_balancer").get(),
    received_packet, ncs.list);
  // Size of Load Balancer packer: lb_packet->getData().size()
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  util.finalizePacket(lb_packet, hdrList);
  // Pushing load balancing packet for table update - 02

  outgoing_packets.push(lb_packet);
  return serverID;
}
// Behavioral methods
void UDPServer::assignServer(std::string dnsreply) {
  TestbedUtilities util;
  // All our DNS packets will be UDP packets for the sake of simplicity
  // 1. type = 0 : DNS Query
  // 2. type = 1 : DNS Response
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  hdrList.push_back("dns_t");
  util.getDnsPacket(received_packet, resPacket, 1, hdrList, dnsreply);
  util.finalizePacket(resPacket, hdrList);
  outgoing_packets.push(resPacket);
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  // DNS reply: dnsreply sent to clientID
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = fileResponse;
}
void UDPServer::establishConnection() {
  // Not required for UDP connections
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = fileResponse;
  registerFile();
}
void UDPServer::registerFile() {
  // Keep receiving packets at this state unless we get a fileID request
  // A fileID request is a packet with a payload of size 1 byte
  // We should have a corresponding element in our filesizes vector
  // Once we receive a valid request, update the pendingFileSize vector
  // Then change the state to sendFile
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  size_t pktsize = received_packet->setData().size();

  // Checking for payload of 1 byte
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;

  if (payloadLen == 1) {
    uint8_t filereq = received_packet->setData().back();
    // if (filereq == 129) {
    // } else {
    // }
  } else {
    // We are expecting a file request from this client.
    // Ignoring till we get one.
    return;
  }
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;

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
  util.getResponseHeader(received_packet, resPacket, payloadLen, ncs.list);

  // We can multiply the video encoding rate with the duration to get the
  // total video duration
  int32_t fileSize = ncs.fsize.size_values.at(cdet->fileIndex);

  uint32_t fsTemp = (uint32_t)fileSize;
  uint8_t *fsptr = static_cast<uint8_t*>(static_cast<void*>(&fsTemp));
  resPacket->setData().insert(resPacket->setData().end(), fsptr, fsptr+4);

  util.finalizePacket(resPacket, ncs.list);
  // Server sending packet. File Size: fileSize
  outgoing_packets.push(resPacket);

  cdet->received_header.clear();
  cdet->received_header.insert(cdet->received_header.begin(),
    received_packet->setData().begin(),
    received_packet->setData().begin()+headerLen);
  cdet->connection_state = fileProcessing;
  cdet->file_pending = fileSize;
  // client_instances[srcIP] = cdet;
  // In UDP we don't wait for any packet from the client before
  // starting the file transfer...
  processFile();
}
void UDPServer::processFile() {
  // We go for datarateManagement to maintain the configured data rate
  // Send the next packet untill the pending file size becomes zero
  // Then erase the entry from client_instances list
  TestbedUtilities util;
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();

  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");

  int maxPayload = ncs.mtu - util.getHeaderLength(ncs.list)
    + sizeof(ether_header);
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  int actPayload = (cdet->file_pending > maxPayload) ?
        maxPayload:cdet->file_pending;
  if (cdet->file_pending == 0) {
    // Erase the connection details
    // client_instances.erase(srcIP);
    cdet->active = false;
    // Client connection deactivated"
    return;
  } else {
    util.getResponseHeader(received_packet, resPacket, 0, ncs.list);
    util.addPayload(resPacket, actPayload);

    util.finalizePacket(resPacket, ncs.list);
    // Server sending packet. Packet size: resPacket->setData().size()!
    // File left: cdet->file_pending - actPayload
    outgoing_packets.push(resPacket);
  }
  // Calculting idle time after every packet is sent to maintain data rates
  double wait = (ncs.mtu) / (ncs.datarate/8.0);
  cdet->idle_pending = sc_time(wait, SC_SEC);
  cdet->wakeup = cdet->idle_pending + sc_time_stamp();
  cdet->file_pending -= actPayload;
  // The idle state in server is actually the datarateManagement state...
  if (cdet->file_pending <= 0) {
    // Using this stage to make server sessions work
    cdet->connection_state = connectionTeardown;
    cdet->active = false;
    // Deleting server session
    serverSessionsManager();
  } else {
    cdet->connection_state = idle;
    // client_instances[srcIP] = cdet;
  }
}
void UDPServer::teardownConnection() {
  // Not required for UDP connections
}
