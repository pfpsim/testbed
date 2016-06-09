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

#include "./LoadBalancer.h"
#include <string>
#include <vector>
#include <map>
#include <utility>

LoadBalancer::LoadBalancer(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):LoadBalancerSIM(nm,parent,configfile),outlog(OUTPUTDIR+"DNSrecords.csv") {  // NOLINT
  outlog << "LogicalTime,Client,ServerAssigned,VirtualInstances(Load)"
    << endl;
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&LoadBalancer::LoadBalancer_PortServiceThread, this)));
  ThreadHandles.push_back(sc_spawn(
    sc_bind(&LoadBalancer::outgoingPackets_thread, this)));
}

void LoadBalancer::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
}
void LoadBalancer::LoadBalancer_PortServiceThread() {
  // Thread function to service input ports.
  TestbedUtilities util;
  while (true) {
    auto received_packet = in->get();
    if (std::dynamic_pointer_cast<TestbedPacket>(received_packet)) {
      rcvd_testbed_packet =
        std::dynamic_pointer_cast<TestbedPacket>(received_packet);
      std::vector<std::string> headers;
      headers.push_back("ethernet_t");
      headers.push_back("ipv4_t");
      size_t headerLen = util.getHeaderLength(headers);
      struct udphdr *udpptr = (struct udphdr*)
        (rcvd_testbed_packet->getData().data() + headerLen);
      uint16_t dport = ntohs(udpptr->uh_dport);
      // DNS Requests are sent to port 53 or a table update request?
      if (dport == 53) {
        invokeDNS();
      } else {
        updateServerSessionsTable();
      }
    }
  }
}
void LoadBalancer::LoadBalancerThread(std::size_t thread_id) {
  // Thread function for module functionalty
}
std::string LoadBalancer::getServerInstanceAddress() {
  TestbedUtilities util;

  std::vector<std::string> headers = util.getPacketHeaders(rcvd_testbed_packet);

  std::vector<std::uint8_t> question;
  question.insert(question.begin(),  (rcvd_testbed_packet->setData().begin() +
    util.getHeaderLength(headers)), (rcvd_testbed_packet->setData().end() - 4));

  char buffer[255];
  for (uint16_t index = 0; index < question.size(); index++) {
    uint16_t letters = question.at(index);
    if (letters == 0) {
      index--;
      buffer[index + letters] = '\0';
      break;
    }
    for (uint16_t j = index + 1 ; j <= index+letters; j++) {
      buffer[j-1] = question.at(j);
    }
    buffer[index + letters] = '.';
    index = index + letters;
  }
  std::string serverURL = std::string(buffer);
  return serverURL;
}
void LoadBalancer::outgoingPackets_thread() {
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(outgoing_packets.pop());
    if (!out->nb_can_put()) {
      gotStuck = true;
    }
    out->put(packet);
    if (gotStuck) {
      gotStuck = false;
    }
  }
}
void LoadBalancer::invokeDNS() {
  TestbedUtilities util;
  std::vector<std::string> headers = util.getPacketHeaders(rcvd_testbed_packet);
  std::string serverURL = getServerInstanceAddress();
  std::pair<
    std::multimap<std::string, instance_infotype >::iterator,
    std::multimap<std::string, instance_infotype >::iterator>
    ppp = server_sessions_table.equal_range(serverURL);

  std::string serverIP;
  int serverLoad = -1;
  std::string output = "DNS: ";
  output.append(std::to_string(server_sessions_table.count(serverURL)));
  output.append(" instances for ");
  output.append(serverURL);
  output.append("[");
  std::string tempLogger;
  tempLogger.append("[");
  for (std::multimap<std::string,
    instance_infotype >::iterator iter = ppp.first;
    iter != ppp.second; ++iter) {
    instance_infotype instance_values = (*iter).second;
    if (serverLoad == -1) {
      serverIP = instance_values.first;
      serverLoad = instance_values.second;
    } else if (serverLoad > instance_values.second) {
      serverIP = instance_values.first;
      serverLoad = instance_values.second;
    }
  }
  std::shared_ptr<TestbedPacket> resPacket =
    std::make_shared<TestbedPacket>();
  util.getDnsPacket(rcvd_testbed_packet, resPacket, 1, headers, serverIP);
  util.finalizePacket(resPacket, headers);
  outgoing_packets.push(resPacket);


  // Update the server_sessions_table instance value
  for (std::multimap<std::string,
    instance_infotype >::iterator iter = ppp.first;
    iter != ppp.second; ++iter) {
    instance_infotype instance_values = (*iter).second;
    if (serverIP.compare(instance_values.first) == 0) {
      iter->second.second++;
      break;
    }
  }
  for (std::multimap<std::string,
    instance_infotype >::iterator iter = ppp.first;
    iter != ppp.second; ++iter) {
    output.append(iter->second.first);
    output.append("(");
    output.append(std::to_string(iter->second.second));
    output.append("); ");
    tempLogger.append(iter->second.first);
    tempLogger.append("(");
    tempLogger.append(std::to_string(iter->second.second));
    tempLogger.append("); ");
  }
  tempLogger.append("]");
  outlog << sc_time_stamp().to_default_time_units() << ","
    << util.getIPAddress(rcvd_testbed_packet->getData(), headers, "src")
    << "," << serverIP << ","
    // << serverURL << ","
    << tempLogger << endl;
  output.append("]");
  npulog(profile, cout << output << endl;)
}
void LoadBalancer::updateServerSessionsTable() {
  TestbedUtilities util;
  std::vector<std::string> headers = util.getPacketHeaders(rcvd_testbed_packet);
  size_t urlLen = 0;
  uint32_t headerLen = util.getHeaderLength(headers);
  std::vector<uint8_t> temp_vector;
  temp_vector.insert(temp_vector.begin(),
    rcvd_testbed_packet->setData().begin() + headerLen,
    rcvd_testbed_packet->setData().end());
  char buffer[255];
  for (uint16_t index = 0; index < temp_vector.size(); index++) {
    uint16_t letters = temp_vector.at(index);
    if (letters == 0) {
      index--;
      buffer[index + letters] = '\0';
      urlLen = index+letters;
      break;
    }
    for (uint16_t j = index + 1 ; j <= index+letters; j++) {
      buffer[j-1] = temp_vector.at(j);
    }
    buffer[index + letters] = '.';
    index = index + letters;
  }
  std::string node_id = std::string(buffer);
  urlLen += 4 - (urlLen % 4);
  temp_vector.clear();
  uint32_t instances_count = ntohl(*(static_cast<uint32_t*>(
    static_cast<void*>(rcvd_testbed_packet->setData().data() + headerLen
    + urlLen))));
  // So first, I erase all entries corresponding to this node ID, and then I
  // reinsert those values so that controller is updated with the server
  server_sessions_table.erase(node_id);
  std::string output;
  output.append("Update ");
  output.append(node_id);
  output.append("[");
  size_t iid_load_len = sizeof(struct in_addr) + sizeof(uint32_t);
  for (size_t index = 0; index < instances_count; index++) {
    struct in_addr *tempip;
    size_t pos = headerLen + urlLen + sizeof(uint32_t)
      + index * iid_load_len;
    tempip = static_cast<struct in_addr*>(static_cast<void*>(
      rcvd_testbed_packet->setData().data() + pos));
    std::string instance_id = inet_ntoa(*tempip);
    uint32_t instance_load = ntohl(*(static_cast<uint32_t*>(
      static_cast<void*>(rcvd_testbed_packet->setData().data() + pos
      + sizeof(struct in_addr)))));
    // I want a server_sessions_table: node_id, instance_id, load
    instance_infotype instance_values(instance_id, instance_load);
    server_sessions_table.insert(
      std::pair<std::string,
      instance_infotype >(node_id, instance_values));
    output.append(instance_id);
    output.append("(");
    output.append(std::to_string(instance_load));
    output.append("); ");
  }
  output.append("]");
  npulog(profile, cout << output << endl;)
}
