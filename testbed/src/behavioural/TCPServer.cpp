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

#include "./TCPServer.h"
#include <string>
#include <utility>
#include <map>
#include <vector>

TCPServer::TCPServer(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):TCPServerSIM(nm,parent,configfile) {  //  NOLINT
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
    (&TCPServer::validatePacketSource_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TCPServer::datarateManager_thread, this)));
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
void TCPServer::initializeServer() {
  TestbedUtilities util;
  npulog(profile, cout << "Creating a new session" << endl;)
  std::string serverID =
    util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
  npulog(profile, cout << "Add server instance: " << serverID << endl;)
  server_sessions.insert(std::pair<std::string, size_t>(serverID, 0));

  std::shared_ptr<TestbedPacket> lb_packet =
    std::make_shared<TestbedPacket>();
  util.getLoadBalancerPacket(lb_packet, server_sessions,
    GetParameter("URL").get(), GetParameter("dns_load_balancer").get(),
    received_packet, ncs.list);
  // SimulationParameters["dns_load_balancer"].get()
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  util.finalizePacket(lb_packet, hdrList);
  npulog(profile, cout << "Pushing load balancing packet for table update - 01"
    << endl;)
  outgoing_packets.push(lb_packet);
}
void TCPServer::populateLocalMap() {
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
void TCPServer::validatePacketSource_thread() {
  while (true) {
    // If we don't get any packet for an hour,
    // we would infer that the clients are all
    // terminated. And hence, we terminate the server also.
    sc_time maxINwait = sc_time(3600, SC_SEC);
    if (!in->nb_can_get()) {
      sc_time stTime = sc_time_stamp();
      wait(maxINwait, in->ok_to_get());
      sc_time endTime = sc_time_stamp();
      if (endTime - stTime >= maxINwait) {
        for (sc_process_handle temp : ThreadHandles) {
          npulog(profile, cout << "tcp server killing all its threads"
          << endl;)
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
    // else, execute the method to establish TCP connection
    struct ConnectionDetails cdet;
    std::string dnsreply;
    if (client_instances.find(clientID) == client_instances.end()) {
      npulog(profile, cout << "Server got new request from client: "
        << clientID << endl;)
     npulog(minimal, cout << "Server got new session request from client: "
        << clientID << endl;)
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
        npulog(profile, cout << "Reactivating an old connection" << endl;)
        npulog(minimal, cout << "Reactivating an old session"
          << endl;)
        cdet.connection_state = connectionSetup;  // serverQuery;
        cdet.file_pending = 0;
        cdet.active = true;
        client_instances[clientID] = cdet;
        npulog(profile, cout << "Prev file index was: " << cdet.fileIndex
        << endl;)
        dnsreply = serverSessionsManager();
      } else {
        npulog(profile, cout << "Continuation of an active connection"
          << endl;)
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
    std::dynamic_pointer_cast<TestbedPacket>(outgoing_packets.pop());
    if (!out->nb_can_put()) {
      npulog(profile, cout << "Server stuck at MUX Ingress! This is bad!"
      << endl;)
      gotStuck = true;
    }
    out->put(packet);
    if (ncs.archive) {
      pcap_logger->logPacket(packet->setData(), sc_time_stamp());
    }
    if (gotStuck) {
      npulog(profile, cout << "Server resumed packet flow to ingress of MUX"
      << endl;)
      gotStuck = false;
    }
  }
}
void TCPServer::datarateManager_thread() {
  // Not needed for TCP Servers for now
}
std::string TCPServer::serverSessionsManager() {
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
  npulog(profile, cout << "ReceivedPacket serverID : " << serverID << endl;)
  std::vector<std::string> baseIPs = util.getBaseIPs(ncs.prefixes);
  if (std::find(baseIPs.begin(), baseIPs.end(), serverID) == baseIPs.end()) {
    npulog(profile, cout << "The serverID does not belong to any of the base "
      << "IPs" << endl;)
    if (server_sessions[serverID] == maxSessions - 1) {
      npulog(profile, cout << "WARNING: The server "<< serverID
        << " instance is reaching its capacity - "
        << static_cast<int>(server_sessions[serverID])
        << " active sessions!" << endl;)
      npulog(profile, cout << "Creating a new session" << endl;)
      std::string newServerID =
        util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
      npulog(profile, cout << "Add server instance: " << newServerID << endl;)
      server_sessions.insert(std::pair<std::string, size_t>(newServerID, 0));
    }
  } else {
    npulog(profile, cout << "Packet was received by the virtual server"
      << endl;)
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
      npulog(profile, cout << "Creating a new session" << endl;)
      serverID =
        util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
      npulog(profile, cout << "Add server instance: " << serverID << endl;)
      server_sessions.insert(std::pair<std::string, size_t>(serverID, 0));
    }
  }
  // If the current state of the received packet is serverQuery,
  // we will increase the server load
  // If the current state of the received packet is connectionTeardown,
  // we will decrease the server load
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  npulog(profile, cout << "Number of client instances"
    << client_instances.size() << endl;)
  if (client_instances.find(clientID) == client_instances.end()) {
    npulog(profile, cout << "The client instance was not found??" << endl;)
    assert(false);
  }
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  npulog(profile, cout << "Connection state is: " << cdet->connection_state
    << endl;)
  if (cdet->connection_state == connectionSetup ||
    cdet->connection_state == serverQuery) {  // serverQuery) {
    npulog(profile, cout << "incrementing server_sessions count" << endl;)
    server_sessions[serverID]++;
  } else if (cdet->connection_state == connectionTeardown) {
    npulog(profile, cout << "decrementing server_sessions count" << endl;)
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
      npulog(profile, cout << "Erase server instance: " << sid << endl;)
      server_sessions.erase(sid);
      }
  }

  // Creating the load balancer packet after all the updates
  // we use the 0th base IP as our node ID
  std::shared_ptr<TestbedPacket> lb_packet =
    std::make_shared<TestbedPacket>();
  npulog(profile, cout << "Number of server sessions: " <<
    server_sessions.size() << endl;)
  util.getLoadBalancerPacket(lb_packet, server_sessions,
    GetParameter("URL").get(), GetParameter("dns_load_balancer").get(),
    received_packet, ncs.list);
  npulog(profile, cout << "Size of Load Balancer packer: "
    << lb_packet->getData().size() << endl;)
  std::vector<std::string> hdrList;
  hdrList.push_back("ethernet_t");
  hdrList.push_back("ipv4_t");
  hdrList.push_back("udp_t");
  util.finalizePacket(lb_packet, hdrList);
  npulog(profile, cout << "Pushing load balancing packet for table update - 02"
    << endl;)
  outgoing_packets.push(lb_packet);
  return serverID;
}
// Behavioral methods
void TCPServer::assignServer(std::string dnsreply) {
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
  npulog(profile, cout << "DNS reply: " << dnsreply << " sent to " << clientID
    << endl;)
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = connectionSetup;
}
void TCPServer::establishConnection() {
  // The received packet must have the SYN flag set
  // Create a random server sequence number and send the ACK/SYN packet
  // Change state of server for this source to sendFileSize
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  std::vector<uint8_t> tcpvec = util.getLayer4Header(
    received_packet->getData(), ncs.list);
  struct tcphdr* tcpheader = (struct tcphdr*)(tcpvec.data());
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  if (tcpheader->th_flags == TH_SYN) {
    std::shared_ptr<TestbedPacket> resPacket
    = std::make_shared<TestbedPacket>();
    util.getResponseHeader(received_packet, resPacket, -1, ncs.list);
    npulog(profile, cout << "ReceivedPacket size: "
    << received_packet->setData().size() << "! Response packet size: "
    << resPacket->setData().size() << endl;)

    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Server sending packet. ACK/SYN response. Size: "
    << resPacket->setData().size() << endl;)
    outgoing_packets.push(resPacket);
    cdet->connection_state = fileResponse;
    // client_instances[srcIP] = cdet;
  }
}
void TCPServer::registerFile() {
  // Keep receiving packets at this state unless we get a fileID request
  // A fileID request is a packet with a payload of size 1 byte
  // We should have a corresponding element in our filesizes vector
  // Once we receive a valid request, update the pendingFileSize vector
  // Then change the state to sendFile
  TestbedUtilities util;
  size_t pktsize = received_packet->setData().size();
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  // Checking for payload of 1 byte
  int headerLen = util.getHeaderLength(ncs.list);
  int payloadLen = pktsize - headerLen;
  if (payloadLen == 1) {
    int fileID = 0;
    fileID = static_cast<int>(received_packet->setData().back());
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
  npulog(profile, cout << "Server sending packet. File Size: " << fileSize
  << endl;)
  outgoing_packets.push(resPacket);

  cdet->connection_state = fileProcessing;
  cdet->file_pending = fileSize;
  // client_instances[srcIP] = cdet;
}
void TCPServer::processFile() {
  // Send the next packet untill the pending file size becomes zero
  // Then change the state to connectionTeardown
  TestbedUtilities util;
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();

  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  int maxPayload = ncs.mtu - util.getHeaderLength(ncs.list)
    + sizeof(ether_header);
  std::vector<uint8_t> tcpvec = util.getLayer4Header(
    received_packet->getData(), ncs.list);
  struct tcphdr* tcpheader = (struct tcphdr*)(tcpvec.data());
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  int actPayload = (cdet->file_pending > maxPayload)
    ? maxPayload:cdet->file_pending;
  if (cdet->file_pending == 0 && tcpheader->th_flags == TH_ACK) {
      npulog(profile, cout << "Server received ACK for final file packet. "
      << "Changing serverstate to connectionTearDown" << endl;)
      cdet->connection_state = connectionTeardown;
  } else {
    // Expecting only ACK packets at this stage
    int headerLen = util.getHeaderLength(ncs.list);
    int pktsize = received_packet->setData().size();
    int payloadLen = pktsize - headerLen;
    if (payloadLen != 0) {
      npulog(profile, cout << "Received a non zero payload during file "
      << "transmission!" << endl;)
      return;
      // we are assuming will be the ack for the previously send packet.
    }
    util.getResponseHeader(received_packet, resPacket, payloadLen, ncs.list);
    util.addPayload(resPacket, actPayload);

    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Server sending packet. Packet size: "
    << resPacket->getData().size() << "! File left: "
    << cdet->file_pending - actPayload << endl;)
    outgoing_packets.push(resPacket);
  }
  cdet->file_pending -= actPayload;
  // client_instances[srcIP] = cdet;
}
void TCPServer::teardownConnection() {
  // Send the FIN/ACK request
  // Once you get the ACK for the FIN, clear up the source connection details
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  std::vector<uint8_t> tcpvec = util.getLayer4Header(
    received_packet->getData(), ncs.list);
  struct tcphdr* tcpheader = (struct tcphdr*)(tcpvec.data());
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();
  if (tcpheader->th_flags == (TH_FIN | TH_ACK)
    || tcpheader->th_flags == TH_FIN) {
    util.getResponseHeader(received_packet, resPacket, -3, ncs.list);
    util.finalizePacket(resPacket, ncs.list);
    npulog(profile, cout << "Server sending packet. ACK/FIN " << endl;)
    outgoing_packets.push(resPacket);
  } else if (tcpheader->th_flags == TH_ACK) {
    npulog(profile, cout << "Server got teardown ACK" << endl;)
    // client_instances.erase(srcIP);
    cdet->active = false;
  } else if (tcpheader->th_flags == TH_RST
    || tcpheader->th_flags == (TH_RST | TH_ACK)) {
    npulog(profile, cout << "Server received RST packet. "
    << "Closing connection" << endl;)
    // client_instances.erase(srcIP);
    cdet->active = false;
  }
  // std::string serverID = util.getIPAddress(received_packet->getData(),
  //  ncs.list, "dst");
  // if (server_sessions.find(serverID) != server_sessions.end()) {
  //  server_sessions[serverID]--;
  npulog(profile, cout << "Deleting server session" << endl;)
  npulog(minimal, cout << "TCPServer: " << "Deleting server session" << endl;)
  serverSessionsManager();
//  cout << "TCPServer: " << "Total number of sessions in the server node: "
//    << server_sessions.size() << endl;
  for (std::map<std::string, size_t>::iterator it = server_sessions.begin();
    it != server_sessions.end(); ++it) {
//  cout << "TCPServer: " << "session: " << it->first << ":"
//    << it->second << endl;
  }

  //} else {
  //  std::cout << "Server session for an unavailable session " + serverID +
  //    " is being processed!";
  //  assert(false);
  //}
}
