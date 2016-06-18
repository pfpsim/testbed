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

TCPClient::TCPClient(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):TCPClientSIM(nm,parent,configfile),outlog(OUTPUTDIR+"TCPClientsRequestResponse.csv") {  // NOLINT
  std::istringstream cf(configfile);
  TestbedUtilities util;
  ncs = util.getClientConfigurations(this, configfile);
  if (ncs.archive) {
    std::string full_name;
    getline(cf, full_name, '.');
    full_name.append("_client.pcap");
    pcap_logger = std::make_shared<PcapLogger>(full_name.c_str());
  }
  // Log information regarding client requests and responses
  outlog << "Client,LogicalTime,Request/Response" << endl;
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
// Administrative methods
void TCPClient::addClientInstances() {
  TestbedUtilities util;
  for (std::vector<uint8_t> header : ncs.header_data) {
    std::string clientID = util.getIPAddress(header, ncs.list, "src");
    npulog(debug, cout << "Adding client instance: "
      << clientID << endl;)
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
void TCPClient::activateClientInstance_thread() {
  int maxInstances = SimulationParameters["NODE"]["instances"].get();
  if (maxInstances == 0) {
    // Configuration specifies zero live instances!
    return;
  }
  if (ncs.end_time.to_double() == 0.0) {
    // Configuration specifies zero life time

    return;
  }
  size_t maxHeaderIndex = ncs.header_data.size();
  size_t delayLen = ncs.connection_delay.delay_values.size();
  if (maxInstances > maxHeaderIndex) {
    std::cerr << "FATAL ERROR: Check the configuration file. Maximum instances"
    << " cannot be more than number of headers" << endl;
    assert(false);
  }
  TestbedUtilities util;
  while (true) {
    // Should we continue the packet generation?
    if (ncs.end_time.to_double() == 0.0 || ncs.end_time <= sc_time_stamp()) {
      // Simulation done for specified time! No more
      // client instances will be activated! Waiting for exisiting processing
      // to end.
      return;
    }
    // size_t activeClients = 0;
    for (std::map<std::string, struct ConnectionDetails>::iterator it
    = client_instances.begin(); it != client_instances.end(); ++it) {
      if (!it->second.active) {
        npulog(debug, cout << "Activating client instance: "
          << it->first << endl;)
        // Activating a client Instance.
        // Either add or activate a instance from allClientDetails map  and
        // send request for TCP file transfer
        if (ncs.connection_delay.distribution.type.compare(
          "round_robin") == 0) {
         it->second.delayIndex++;
         if (it->second.delayIndex == delayLen) {
           it->second.delayIndex = 0;
         }
        } else {
          it->second.delayIndex = util.getRandomNum(0, delayLen - 1,
            ncs.connection_delay.distribution.type,
            ncs.connection_delay.distribution.param1,
            ncs.connection_delay.distribution.param2);
        }
        sc_time waittime =
          ncs.connection_delay.delay_values.at(it->second.delayIndex);
        // Adding client instance with idle delay of: waittime
        it->second.connection_state = serverQuery;
        it->second.idle_pending = waittime;
        it->second.file_pending = 0;
        it->second.active = true;
        // Activating: it->first
        // Initiate sending of the SYN packet
        received_packet = NULL;
        npulog(debug, cout << "Invoking DNS for " << it->first << endl;)
        acquireServerInstance(it->first);
        // We activated a client instance. Waiting for it to connect!
        npulog(debug, cout
          << "Waiting for the client to finish connection before we "
          << "activate the next client" << endl;)
        wait(activate_client_instance_event);
        npulog(debug,
          cout << "Client connected successully, we can activate next client"
            << endl;)
        // establishConnection(it->first);
      }
    }
    // Wait for a client instance to be activated
    // A client instance will be activated if *timed out*
    // or after its TCP file transfer is done
    // We activated the specified number of client instances.
    // Waiting for next activation request!
    npulog(debug,
      cout << "Required client instances have been activated. Waiting for "
        << "next round of connections" << endl;)
    wait(reactivate_client_instance_event);
    npulog(debug,
      cout << "Notification for termination of a client instance received"
        <<endl;)
    // reactivate_client_instance_event notified.
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
  if (client_instances.empty()) {
    // This thread cannot be executed like this :)
    wait(SC_ZERO_TIME);
  }
  int maxInstances = SimulationParameters["NODE"]["instances"].get();
  if (maxInstances == 0) {
    // Configuration specifies zero live instances!
    return;
  } else if (ncs.end_time.to_double() == 0.0) {
    // Configuration specifies zero life
    return;
  }
  double rtime = 1;
  sc_time_unit runit = SC_NS;
  while (true) {
    if (ncs.end_time.to_double() == 0.0) {
      // Simulation done for specified time! All files end!
      // Scheduler ends because of zero time
      return;
    }
    if (ncs.end_time <= sc_time_stamp()) {
        // Simulation done for specified time! All files end!
        // Scheduler ends because of specified time exceeded
        return;
    }
    int idleInstances = 0;
    bool clWakeup = false;
    sc_time minTime;
    std::string minTimeCID;
    for (std::map<std::string, ConnectionDetails>::iterator it =
      client_instances.begin(); it != client_instances.end(); ++it) {
      if (it->second.connection_state == idle) {
        npulog(debug, cout << it->first
          << " is in idle status" << endl;)
        if (it->second.wakeup <= sc_time_stamp()) {
          it->second.active = false;
          it->second.connection_state = serverQuery;
          // Re-activating client instance for the finished file. Wakeup!
          npulog(debug, cout << "wake up call for " << it->first << endl;)
          reactivate_client_instance_event.notify();
          // Allow the client thread to be instantiated immediately by yielding
          wait(SC_ZERO_TIME);
          clWakeup = true;
          rtime = 1;
          runit = SC_NS;
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
      }
    }
    if (!clWakeup) {
      if (idleInstances == client_instances.size() &&
        !client_instances.empty()) {
        // All client instances are idle[idleInstances]!
        // Going for a wait now for minTime
        npulog(debug, cout << "All client instances are idle. "
          << "Going for a wait now for minTime" << minTime << endl;)
        wait(minTime);
        npulog(debug, cout << minTimeCID
          << " has been identified as the least idle time"
          << ". We have waited for the time" << endl;)
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
        npulog(debug, cout << "For "
          << minTimeCID << "we send the reactivate_client notify"
          << endl;)
        reactivate_client_instance_event.notify();
        wait(SC_ZERO_TIME);
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
              // Do nothing
              npulog(debug, cout << "lala land" << endl;)
          }
        }
        sc_time polling_time = sc_time(rtime, runit);
        wait(polling_time);
        rtime = rtime*2;
        // tcp client is uncontrolled!
      }
    }
  }
}
void TCPClient::outgoingPackets_thread() {
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoing_packets.pop());
    if (!out->nb_can_put()) {
      assert(!"Client got stuck at Mux input! FATAL. Increase fifo size?");
      gotStuck = true;
    }
    npulog(debug, cout << "Sending packet from client to server" << endl;)
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
void TCPClient::validatePacketDestination_thread() {
  while (true) {
    received_packet = std::dynamic_pointer_cast<TestbedPacket>(in->get());
    TestbedUtilities util;
    std::string clientID = util.getIPAddress(received_packet->getData(),
      ncs.list, "dst");
      npulog(debug, cout << clientID << " received a packet" << endl;)
    struct ConnectionDetails cdet;
    if (client_instances.find(clientID) == client_instances.end()) {
      // Strange! We got a packet we have nothing to do with! Ignored
      npulog(debug,
        cout << "Strange! We got a packet we have nothing to do with! Ignored"
          << endl;)
      continue;
    } else {
      cdet = client_instances.find(clientID)->second;
    }
    switch (cdet.connection_state) {
      case serverQuery:
        npulog(debug, cout << clientID << "in serverQuery state" << endl;)
        acquireServerInstance("0");
        break;
      case connectionSetup:
        // Passing null because once we have a received_packet,
        // the actual headerIndex does not matter
        npulog(debug, cout << clientID << "in connectionSetup state" << endl;)
        establishConnection("0", "0");
        break;
      case fileRequest:
        npulog(debug, cout << clientID << "in fileRequest state" << endl;)
        requestFile();
        break;
      case fileResponse:
        npulog(debug, cout << clientID << "in fileResponse state" << endl;)
        registerFile();
        break;
      case fileProcessing:
        npulog(debug, cout << clientID << "in fileProcessing state" << endl;)
        processFile();
        break;
      case connectionTeardown:
        npulog(debug, cout << clientID << "in connectionTeardown state"
          << endl;)
        teardownConnection();
        break;
      case idle:
        // nothing to do actually, this is handled by a thread
        // If we get more packets here, I believe they will simply be
        // rejected at this stage :D
        npulog(debug, cout << clientID << "in idle state" << endl;)
        break;
    }
  }
}
void TCPClient::acquireServerInstance(std::string clientID) {
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
      SimulationParameters["NODE"]["serverURL"].get());
    util.finalizePacket(resPacket, hdrList);
    // Client clientID sends DNS request
    npulog(debug, cout << "Client " << clientID << " sends DNS request"
      << endl;)
    outgoing_packets.push(resPacket);
  } else {
    std::string serverID = util.getDNSResponseIPAddr(received_packet, hdrList);
    clientID = util.getIPAddress(received_packet->getData(),
      hdrList, "dst");
    // Client received DNS response.
    npulog(profile, cout << Red << clientID << " is assigned with " << serverID
      << txtrst << endl;)
    // outlog << sc_time_stamp().to_default_time_units() << ","
    //  << clientID << ","
    //  << serverID << endl;  // NOLINT
    struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
    cdet->connection_state = connectionSetup;
    received_packet = NULL;
    npulog(debug, cout << "Client " << clientID << " received DNS response. "
      << "Going for establish connection with " << serverID << endl;)
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
  if (received_packet == NULL) {
    // Send the SYN packet
    std::shared_ptr<TestbedPacket> reqPacket =
      std::make_shared<TestbedPacket>();
    struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
    reqPacket->setData().insert(reqPacket->setData().begin(),
        cdet->received_header.begin(),
        cdet->received_header.end());
    util.updateAddress(reqPacket, ncs.list, serverID, "dst");
    // Client util.getIPAddress(reqPacket->getData(), ncs.list, "src")
    // header updated with received server address
    // util.getIPAddress(reqPacket->getData(), ncs.list, "dst")
    // clientID sending SYN packet to serverID!
    util.finalizePacket(reqPacket, ncs.list);
    npulog(debug, cout << clientID << "sending syn packet to " << serverID
      << endl;)
    outgoing_packets.push(reqPacket);
  } else {
    // Send ACK packet and file request packet and
    // then change state to sendFileRequest
    std::shared_ptr<TestbedPacket> reqPacket
    = std::make_shared<TestbedPacket>();
    util.getResponseHeader(received_packet, reqPacket, -2, ncs.list);
    util.finalizePacket(reqPacket, ncs.list);
    // Client sending ACK packet
    npulog(debug, cout << clientID << "sending ACK for connection establish to "
      << serverID << endl;)
    outgoing_packets.push(reqPacket);
    // Client connection established. Next client to be instantiated now!
    npulog(debug, cout << clientID
      << " connected and now notifying the next client" << endl;)
    activate_client_instance_event.notify();
    npulog(debug, cout << clientID << "requesting file from server "
      << serverID << endl;)
    requestFile();
  }
}
void TCPClient::requestFile() {
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  std::shared_ptr<TestbedPacket> reqPacket = std::make_shared<TestbedPacket>();
  util.getResponseHeader(received_packet, reqPacket, -2, ncs.list);
  reqPacket->setData().push_back(129);
  util.finalizePacket(reqPacket, ncs.list);
  // Client sending file request packet"
  outgoing_packets.push(reqPacket);
  npulog(debug, cout << clientID << " sending request for file transfer."
    << endl;
  outlog << clientID << "," << sc_time_stamp().to_default_time_units() << ","
    << "request" << endl;)
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = fileResponse;
}
void TCPClient::registerFile() {
  // We get a packet with a payload of 4 bytes
  // Store the value of the payload in the cdet structure
  // Send ACK for the received file size and change state to fileTransfer
  TestbedUtilities util;
  size_t pktsize = received_packet->setData().size();
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  // Checking for payload of 1 byte
  size_t headerLen = util.getHeaderLength(ncs.list);
  size_t payloadLen = pktsize - headerLen;
  size_t fileSize = 0;
  if (payloadLen == 4) {
    outlog << clientID << "," << sc_time_stamp().to_default_time_units() << ","
      << "response" << endl;
      npulog(debug, cout << clientID << " received file details from the server"
        << endl;)
    uint32_t *fsptr = static_cast<uint32_t*>
      (static_cast<void*>(received_packet->setData().data() + headerLen));
    fileSize = *fsptr;
  } else {
    // We are expecting a file size of 4 bytesfrom the server
    // Ignoring till we get one.
    return;
  }
  // sending ACK packet for the received filesize
  std::shared_ptr<TestbedPacket> resPacket =
  std::make_shared<TestbedPacket>();
  util.getResponseHeader(received_packet, resPacket, payloadLen, ncs.list);
  util.finalizePacket(resPacket, ncs.list);
  // Client sending ACK for received fileSize
  npulog(debug, cout << clientID
    << " sending ack for the received file of size: " << fileSize
    << endl;)
  outgoing_packets.push(resPacket);
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
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
  size_t pktsize = received_packet->setData().size();
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  int headerLen = util.getHeaderLength(ncs.list);
  int payloadLen = pktsize - headerLen;
  // sending ACK packet for the received file segement
  std::shared_ptr<TestbedPacket> reqPacket = std::make_shared<TestbedPacket>();
  util.getResponseHeader(received_packet, reqPacket, -2, ncs.list);
  util.finalizePacket(reqPacket, ncs.list);
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->file_pending -= payloadLen;
  // Client sending ACK for received fileSize. Pending payload:
  // cdet->file_pending
  npulog(debug, cout << clientID
    << " received packet with file contents of size: "
    << payloadLen << ". Remaining file: " << cdet->file_pending << endl;)
  outgoing_packets.push(reqPacket);
  // Simply calling the teardown Connection to send the RST packet_data_type
  if (cdet->file_pending <= 0) {
    npulog(debug, cout << clientID
      << " will begin the connection teardown" << endl;)
    cdet->connection_state = connectionTeardown;
    teardownConnection();
  } else {
    cdet->connection_state = fileProcessing;
  }
}
void TCPClient::teardownConnection() {
  // We are using RST to close the connection//Normally this method is
  // used to perform the handshaking signals
  TestbedUtilities util;
  std::string clientID = util.getIPAddress(received_packet->getData(),
    ncs.list, "dst");
  std::shared_ptr<TestbedPacket> reqPacket
    = std::make_shared<TestbedPacket>();
  util.getResponseHeader(received_packet, reqPacket, -5, ncs.list);
  util.finalizePacket(reqPacket, ncs.list);
  // Client sending RST to close the connection."
  npulog(debug, cout << clientID
    << "sent RST packet to server to close connection" << endl;)
  outgoing_packets.push(reqPacket);
  // update state to idle and update the wake up time as well
  struct ConnectionDetails *cdet = &client_instances.find(clientID)->second;
  cdet->connection_state = idle;
  cdet->wakeup = cdet->idle_pending + sc_time_stamp();
  // Client clientID will wake up at cdet->wakeup
  npulog(debug, cout << clientID
    << " will now wake up at " << cdet->wakeup << endl;)
}
