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
#include <limits>

UDPServer::UDPServer(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):UDPServerSIM(nm,parent,configfile),se_use_logger(OUTPUTDIR+"TCPServersUtilization.csv"),se_power_logger(OUTPUTDIR+"TCPServerPower.csv"), se_throughput_logger("TCPNodeThroughput.csv") {  //  NOLINT
  std::istringstream cf(configfile);
  TestbedUtilities util;
  ncs = util.getServerConfigurations(this, configfile);
  if (ncs.archive) {
    std::string full_name;
    std::getline(cf, full_name, '.');
    full_name.append("_server.pcap");
    pcap_logger = std::make_shared<PcapLogger>(full_name.c_str());
  }
  // Log information regarding utilization of servers
  se_use_logger << "Server,Client,LogicalTime,Establish/Teardown" << endl;
  // Log information regarding instantiation of servers
  se_power_logger << "Server,LogicalTime" << endl;
  // Log information regarding packets sent out from the server
  se_throughput_logger << "LogicalTime,Server,Client,PacketLength,PacketInfo"
    << endl;
  initializeServer();
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&UDPServer::outgoingPackets_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&UDPServer::datarateManager_thread, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&UDPServer::validatePacketSource_thread, this)));
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
  npulog(debug, cout << "We will be creating a server instance: "
    << serverID << endl;)
  server_sessions.insert(std::pair<std::string, size_t>(serverID, 0));
  se_power_logger << serverID << "," << sc_time_stamp().to_default_time_units()
    << endl;
  std::shared_ptr<TestbedPacket> lb_packet =
    std::make_shared<TestbedPacket>();
  std::vector<std::string> server_instances;
  server_instances.push_back(serverID);
  /*
  std::shared_ptr<TestbedPacket> lb_packet,
  const std::vector<std::string> &server_instances,
  const std::string &public_url,
  const std::string &public_ip,
  const std::string &controller_ip
  */
  //   const std::vector<std::string> &prefixes
  std::string public_ip = util.getBaseIPs(ncs.prefixes.prefix_values).at(0);
  util.getLoadBalancerPacket(lb_packet, server_instances,
    SimulationParameters["NODE"]["serverURL"].get(), public_ip,
    GetParameter("dns_load_balancer").get());
  // SimulationParameters["dns_load_balancer"].get()
  std::vector<std::string> hdrList =
    util.getPacketHeaders(lb_packet->getData());
  util.finalizePacket(lb_packet, hdrList);
  // Pushing load balancing packet for table update - 01"
  npulog(profile,
    cout << Yellow << "Server sending lb_packet" << txtrst << endl;)
  outgoing_packets.push(lb_packet);
}
// Administrative methods
void UDPServer::validatePacketSource_thread() {
  int maxSessions = SimulationParameters["INSTANCE"]["sessions"].get();
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
          // udp server killing all its threads"
          temp.kill();
        }
      }
    }
    received_packet = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getIPAddress(received_packet->getData(),
      ncs.list, "src");
    std::string serverID = util.getIPAddress(received_packet->getData(),
      ncs.list, "dst");
    npulog(debug, cout << serverID << " received a packet from "
      << clientID << endl;)
    // Check if we have an ongoing file with the client
    // If yes, call the method to execute sending the payload
    // else, execute the method to establish UDP connection
    std::string dnsreply;
    if (client_instances.find(clientID) == client_instances.end()) {
      // Server got new request from client: clientID

      // Assess the load on the server instance. If beyond config, just drop
      // this packet
      // This check is actually redundant.. but then, you never know :D
      if (server_sessions.find(serverID) != server_sessions.end()) {
        auto inst = server_sessions.find(serverID);
        if (inst->second >= maxSessions) {
          // We have exceeded/ are at the server capacity despite the threshold!
          // We will drop this connection!
          npulog(profile, cout << Yellow
            << "Server dropped a connection request"
            << "from " << clientID << txtrst << endl;)
          received_packet.reset();

          std::shared_ptr<TestbedPacket> lb_packet =
            std::make_shared<TestbedPacket>();
          std::vector<std::string> mod_server_instances;
          mod_server_instances.push_back(serverID);
          std::string public_ip =
            util.getBaseIPs(ncs.prefixes.prefix_values).at(0);
          util.getLoadBalancerPacket(lb_packet, mod_server_instances,
            SimulationParameters["NODE"]["serverURL"].get(), public_ip,
            GetParameter("dns_load_balancer").get());
          util.finalizePacket(lb_packet,
            util.getPacketHeaders(lb_packet->getData()));
          npulog(profile,
                  cout << Yellow << "Server sending lb_packet" << txtrst
                  << endl;)
          outgoing_packets.push(lb_packet);
          continue;
        }
      }

      npulog(profile, cout << Yellow << "New server session with " << clientID
        << txtrst << endl;)
      struct ConnectionDetails cdet;
      cdet.connection_state = connectionSetup;  // serverQuery;
      cdet.fileIndex = -1;
      cdet.file_pending = 0;
      cdet.active = true;
      client_instances.insert(
        std::pair<std::string, struct ConnectionDetails>(clientID, cdet));
      dnsreply = serverSessionsManager();
    } else {
      struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
      if (cdet->active == false) {
        // Assess the load on the server instance. If beyond config, just drop
        // this packet
        // This check is actually redundant.. but then, you never know :D
        if (server_sessions.find(serverID) != server_sessions.end()) {
          auto inst = server_sessions.find(serverID);
          if (inst->second >= maxSessions) {
            // We have exceeded/ are at the server capacity despite the
            // threshold! We will drop this connection!
            npulog(profile, cout << Yellow
              << "Server dropped a connection request"
              << "from " << clientID << txtrst << endl;)
            received_packet.reset();

            std::shared_ptr<TestbedPacket> lb_packet =
              std::make_shared<TestbedPacket>();
            std::vector<std::string> mod_server_instances;
            mod_server_instances.push_back(serverID);
            std::string public_ip =
              util.getBaseIPs(ncs.prefixes.prefix_values).at(0);
            util.getLoadBalancerPacket(lb_packet, mod_server_instances,
              SimulationParameters["NODE"]["serverURL"].get(), public_ip,
              GetParameter("dns_load_balancer").get());
            util.finalizePacket(lb_packet,
              util.getPacketHeaders(lb_packet->getData()));
            npulog(profile,
                    cout << Yellow << "Server sending lb_packet" << txtrst
                    << endl;)
            outgoing_packets.push(lb_packet);
            continue;
          }
        }


        npulog(profile, cout << Yellow << "New server session with " << clientID
          << txtrst << endl;)
        // Reactivating an old connection
        cdet->connection_state = connectionSetup;  // serverQuery;
        cdet->file_pending = 0;
        cdet->active = true;
        cdet->received_header.clear();
        cdet->received_header.insert(cdet->received_header.begin(),
          received_packet->setData().begin(), received_packet->setData().end());
        // client_instances[clientID] = cdet;
        // Prev file index was: cdet.fileIndex
        dnsreply = serverSessionsManager();
      } else {
        // Continuation of an active connection
        struct ConnectionDetails *cdet =
          &client_instances.find(clientID)->second;
        cdet->received_header.clear();
        cdet->received_header.insert(cdet->received_header.begin(),
          received_packet->setData().begin(), received_packet->setData().end());
      }
    }
    struct ConnectionDetails *cdet =
      &client_instances.find(clientID)->second;
    switch (cdet->connection_state) {
      case serverQuery:
        npulog(debug, cout << serverID << " and " << clientID
          << " are in serverQuery mode" << endl;)
        assignServer(dnsreply);
        break;
      case connectionSetup:
        npulog(debug, cout << serverID << " and " << clientID
          << " are in connectionSetup mode"
          << endl;)
        establishConnection();
        break;
      case fileRequest:
        // Server's do not send requests for files
        break;
      case fileResponse:
        npulog(debug, cout << serverID << " and " << clientID
          << " are in fileResponse mode"
          << endl;)
        registerFile();
        break;
      case fileProcessing:
        npulog(debug, cout << serverID << " and " << clientID
          << " are in fileProcessing mode"
          << endl;)
        processFile();
        break;
      case connectionTeardown:
        // tearConnection();
        break;
      case idle:
        npulog(debug, cout << serverID << " and " << clientID
          << " are in idle mode"
          << endl;)
        updateConnectionState();
        break;
    }
  }
}
void UDPServer::outgoingPackets_thread() {
  bool gotStuck = false;
  TestbedUtilities util;
  const char* ConnectionStateTypes[] = {
    "serverQuery",
    "connectionSetup",
    "fileRequest",
    "fileResponse",
    "fileProcessing",
    "idle",
    "connectionTeardown"
  };
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoing_packets.pop());
    if (!out->nb_can_put()) {
      assert(!"Server stuck at MUX Ingress! This is bad!");
      gotStuck = true;
    }
    // "LogicalTime,Server,Client,PacketLength,PacketInfo"
    std::vector<std::string> headers = util.getPacketHeaders(packet->getData());
    std::string clientID =
      util.getIPAddress(packet->getData(), headers, "dst");
    se_throughput_logger << sc_time_stamp().to_default_time_units() << ","
      << util.getIPAddress(packet->getData(), headers, "src") << ","
      << clientID << ","
      << packet->getData().size() << ",";
    for (std::string hdr : headers) {
      se_throughput_logger << hdr << ";";
    }
    if (client_instances.find(clientID) != client_instances.end()) {
      struct ConnectionDetails cdet = client_instances.find(clientID)->second;
      se_throughput_logger << ConnectionStateTypes[cdet.connection_state];
    } else {
      se_throughput_logger << "NoConnectionsYet(" << clientID << ")!";
    }
    se_throughput_logger << endl;
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
  double rtime = 1;
  sc_time_unit runit = SC_NS;
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
          cdet->connection_state = cdet->next_connection_state;
          processFile();
          rtime = 1;
          runit = SC_NS;
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
        cdet->connection_state = cdet->next_connection_state;
        processFile();
        rtime = 1;
    runit = SC_NS;
      } else {
        // wait for resolution time
        if (rtime > 1000) {
          switch (runit) {
            case SC_NS:
              runit = SC_US;
              rtime = 1;
              break;
            case SC_US:
              runit = SC_MS;
              rtime = 1;
              break;
            case SC_MS:
              runit = SC_SEC;
              rtime = 1;
              break;
            default:
              rtime *= 2;
              if (rtime > std::numeric_limits<double>::max()) {
              npulog(debug, cout << "Reached limit! Exiting" << endl;)
                exit(0);
            }
            // Do nothing
          }
        }
        sc_time polling_time = sc_time(rtime, runit);
        wait(polling_time);
        rtime = rtime*2;
      }
    }
  }
}
std::string UDPServer::serverSessionsManager() {
  TestbedUtilities util;
  bool send_lb_update = false;
  int maxSessions = SimulationParameters["INSTANCE"]["sessions"].get();
  double config_threshold = SimulationParameters["INSTANCE"]["threshold"].get();
  config_threshold = 1 - config_threshold;
  std::string  serverID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  std::string  clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  // This is a list of server instances which have either been created
  // or have experienced a reduce in their number of sessions
  std::vector<std::string> mod_server_instances;
  // ReceivedPacket serverID: serverID
  npulog(debug, cout << "server sessions manager in action for: "
    << serverID << " and " << clientID << endl;)
  // If the current state of the received packet is serverQuery,
  // we will increase the server load
  // If the current state of the received packet is connectionTeardown,
  // we will decrease the server load
  // Number of client instances client_instances.size()
  if (client_instances.find(clientID) == client_instances.end()) {
    // The client instance was not found??
    assert(false);
  }
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  // Connection state is: cdet->connection_state

  if (cdet->connection_state == connectionSetup ||
    cdet->connection_state == fileResponse ||
    cdet->connection_state == serverQuery) {  // serverQuery) {
    // incrementing server_sessions count
    npulog(profile, cout << Yellow << "Incrementing server_sessions count"
      << txtrst << endl;)
    server_sessions[serverID]++;
  } else if (cdet->connection_state == connectionTeardown) {
    // decrementing server_sessions count
    server_sessions[serverID]--;
    send_lb_update = true;
    mod_server_instances.push_back(serverID);
    npulog(debug, cout << Yellow << "Decrementing server_sessions count"
      << txtrst << endl;)
  }
  std::vector<std::string> baseIPs =
    util.getBaseIPs(ncs.prefixes.prefix_values);
  if (std::find(baseIPs.begin(), baseIPs.end(), serverID) == baseIPs.end()) {
    double total_available_sessions = 0;
    double total_sessions = 0;
    double total_active_sessions = 0;
    // The serverID does not belong to any of the base IPs
    // The server serverID instance is reaching its capacity
    // Creating a new session
    // Check for available slots before creating a new server session
    for (std::map<std::string, size_t>::iterator it = server_sessions.begin();
      it != server_sessions.end(); ++it) {
      if (it->second > maxSessions) {
        npulog(debug, cout << "The server " << it->first << "("
          << it->second << ")"
          << " is being overloaded!!" << endl;)
      }
      total_sessions += maxSessions;
      total_active_sessions += it->second;
    }
    total_available_sessions = total_sessions - total_active_sessions;
    std::string outputTemp;
    outputTemp.append("Available sessions(");
    outputTemp.append(std::to_string(total_available_sessions));
    outputTemp.append(")");
    double current_threshold = total_available_sessions / total_sessions;
    npulog(debug, cout << "max sessions: " << maxSessions << endl
      << "number of instances: " << server_sessions.size() << endl
      << "total sessions: " << total_sessions << endl
      << "total_available_sessions: " << total_available_sessions << endl
      << "current threshold: " << current_threshold << endl
      << "configured threshold: " << config_threshold << endl
      << endl;)
    // This is dependant on the configured threshold
    // But 1 is the minimum number of available sessions required no matter what
    if (total_available_sessions <= 1 ||
      current_threshold <= config_threshold) {
      npulog(debug, cout << "Creating a new server instance" << endl;)
      std::string newServerID =
        util.getServerInstanceAddress(ncs.prefixes, server_sessions, 1);
      // Add server instance: newServerID
      server_sessions.insert(std::pair<std::string, size_t>(newServerID, 0));
      mod_server_instances.push_back(newServerID);
      send_lb_update = true;
      se_power_logger << newServerID << ","
        << sc_time_stamp().to_default_time_units() << endl;
      outputTemp.append(" - creating ");
      outputTemp.append(newServerID);
    }
    npulog(profile, cout << Yellow << outputTemp << txtrst << endl;)
    npulog(debug, cout << Yellow << outputTemp << txtrst << endl;)
  } else {
    // We do not receive packets here anymore
    assert(!"Packet sent to server base IP");
  }
  if (send_lb_update) {
    std::shared_ptr<TestbedPacket> lb_packet =
      std::make_shared<TestbedPacket>();
    /*const std::vector<std::string> &server_instances,
    const std::string &public_url,
    const std::string &public_ip,
    const std::string &controller_ip
    */
    //   const std::vector<std::string> &prefixes
    std::string public_ip = util.getBaseIPs(ncs.prefixes.prefix_values).at(0);
    util.getLoadBalancerPacket(lb_packet, mod_server_instances,
      SimulationParameters["NODE"]["serverURL"].get(), public_ip,
      GetParameter("dns_load_balancer").get());
    util.finalizePacket(lb_packet, util.getPacketHeaders(lb_packet->getData()));
    npulog(profile,
            cout << Yellow << "Server sending lb_packet" << txtrst << endl;)
    outgoing_packets.push(lb_packet);
  }
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
  assert(!"We have stopped sending out DNS replies. Duh! Load balancer!");
  outgoing_packets.push(resPacket);
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  // DNS reply: dnsreply sent to clientID
  npulog(debug, cout
    << "Servers are still sending out DNS replies??? WHY!!!" << endl;)
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = fileResponse;
}
void UDPServer::establishConnection() {
  // Not required for UDP connections
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  std::string serverID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");

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
  std::string serverID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  size_t pktsize = received_packet->setData().size();

  // Checking for payload of 1 byte
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;
  npulog(debug, cout << "UDP Server received a packet! Is it a file request?"
    << endl;)
  if (payloadLen == 1) {
    se_use_logger << serverID << "," << clientID << ","
      << sc_time_stamp().to_default_time_units() << "," << "establish" << endl;
    uint8_t filereq = received_packet->setData().back();
    npulog(debug, cout << "Payload is size " << payloadLen
      << " and its value is " << filereq << endl;)
    // if (filereq == 129) {
    // } else {
    // }
  } else {
    // We are expecting a file request from this client.
    // Ignoring till we get one.
    npulog(debug, cout << "Server received a packet of size greater than 1B"
      << endl;)
    return;
  }
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;

  int fileLen = ncs.fsize.size_values.size();
  if (ncs.fsize.distribution.type.compare("round_robin") == 0) {
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
  cout << "processFile " << endl;
  // We go for datarateManagement to maintain the configured data rate
  // Send the next packet untill the pending file size becomes zero
  // Then erase the entry from client_instances list
  TestbedUtilities util;
  std::shared_ptr<TestbedPacket> resPacket = std::make_shared<TestbedPacket>();

  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "src");
  std::string serverID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  npulog(debug, cout << "File processing between " << clientID
    << " and " << serverID << endl;)
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
  double wait = resPacket->getData().size() / (ncs.datarate/8.0);
  // (ncs.mtu) / (ncs.datarate/8.0);
  cout << "Datarate wait is: " << wait << endl;
  cdet->idle_pending = sc_time(wait, SC_SEC);
  cdet->wakeup = cdet->idle_pending + sc_time_stamp();
  cdet->file_pending -= actPayload;
  // The idle state in server is actually the datarateManagement state...
  // Now we also need to add the application delays to this delay :)
  // This delay is dependant on the number of sessions this virtual
  // server instance is handling...
  size_t delayLen = ncs.application_delay.delay_values.size();
  if (ncs.application_delay.distribution.type.compare(
    "round_robin") == 0) {
    cdet->delayIndex++;
    if (cdet->delayIndex == delayLen) {
      cdet->delayIndex = 0;
    }
  } else {
    cdet->delayIndex = util.getRandomNum(0, delayLen - 1,
      ncs.application_delay.distribution.type,
      ncs.application_delay.distribution.param1,
      ncs.application_delay.distribution.param2);
  }
  sc_time application_delay_waittime =
  ncs.application_delay.delay_values.at(cdet->delayIndex);
  size_t active_sessions = server_sessions.find(serverID)->second;
  // This happens for number of sessions beyond 1 :)
  cout << "Application delay wait is: " << application_delay_waittime << endl;
  application_delay_waittime *= (active_sessions - 1);
  cdet->idle_pending += application_delay_waittime;
  cdet->wakeup += application_delay_waittime;

  if (cdet->file_pending <= 0) {
    // Using this stage to make server sessions work
    cdet->connection_state = connectionTeardown;
    cdet->active = false;
    // Deleting server session
    se_use_logger << clientID << "," << sc_time_stamp().to_default_time_units()
      << ","
      << "teardown" << endl;
    serverSessionsManager();
  } else {
    // We will use this to return back after idle phase
    cdet->next_connection_state = cdet->connection_state;
    cdet->connection_state = idle;
    // client_instances[srcIP] = cdet;
  }
}
void UDPServer::teardownConnection() {
  // Not required for UDP connections
}
void UDPServer::updateConnectionState() {
  // Not required for UDP servers
}
