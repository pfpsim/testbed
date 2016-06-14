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
      // DNS Requests are sent to port 53 or a table update request
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

  std::vector<std::string> headers =
    util.getPacketHeaders(rcvd_testbed_packet->getData());

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
  std::vector<std::string> headers =
    util.getPacketHeaders(rcvd_testbed_packet->getData());
  std::string serverURL = getServerInstanceAddress();
  std::pair<
    std::multimap<std::string, instance_infotype >::iterator,
    std::multimap<std::string, instance_infotype >::iterator>
    ppp = server_sessions_table.equal_range(serverURL);

  std::string server_ip;
  std::string public_ip;
  int serverLoad = -1;
  std::string tempLogger;
  tempLogger.append("[");
  for (std::multimap<std::string,
    instance_infotype >::iterator iter = ppp.first;
    iter != ppp.second; ++iter) {
    instance_infotype instance_values = (*iter).second;
    if (serverLoad == -1) {
      server_ip  = std::get<0>(instance_values);
      serverLoad = std::get<1>(instance_values);
      public_ip  = std::get<2>(instance_values);
    } else if (serverLoad > std::get<1>(instance_values)) {
      server_ip  = std::get<0>(instance_values);
      serverLoad = std::get<1>(instance_values);
      public_ip  = std::get<2>(instance_values);
    }
  }
  std::shared_ptr<TestbedPacket> resPacket =
    std::make_shared<TestbedPacket>();
  util.getDnsPacket(rcvd_testbed_packet, resPacket, 1, headers, public_ip);
  util.finalizePacket(resPacket, headers);
  outgoing_packets.push(resPacket);


  std::string client_ip = util.getIPAddress(rcvd_testbed_packet->getData(),
    headers, "src");
  // Currently we are not putting clients into prefixes
  client_ip.append("/32");

  npulog(profile, cout << public_ip << " returned to " << client_ip
    << ". Server instance " << server_ip << endl; )
  updateForwardNAT(client_ip, public_ip, server_ip);
  // Update the server_sessions_table instance value
  for (std::multimap<std::string,
    instance_infotype >::iterator iter = ppp.first;
    iter != ppp.second; ++iter) {
    instance_infotype &instance_values = (*iter).second;
    if (server_ip.compare(std::get<0>(instance_values)) == 0) {
      std::get<1>(instance_values)++;
      break;
    }
  }
  std::string output = "DNS: ";
  output.append(std::to_string(server_sessions_table.count(serverURL)));
  output.append(" instances for ");
  output.append(serverURL);
  output.append("[");
  for (std::multimap<std::string,
    instance_infotype >::iterator iter = ppp.first;
    iter != ppp.second; ++iter) {
    output.append(std::get<0>(iter->second));
    output.append("(");
    output.append(std::to_string(std::get<1>(iter->second)));
    output.append("); ");
    tempLogger.append(std::get<0>(iter->second));
    tempLogger.append("(");
    tempLogger.append(std::to_string(std::get<1>(iter->second)));
    tempLogger.append("); ");
  }
  tempLogger.append("]");
  outlog << sc_time_stamp().to_default_time_units() << ","
    << util.getIPAddress(rcvd_testbed_packet->getData(), headers, "src")
    << "," << server_ip << ","
    // << serverURL << ","
    << tempLogger << endl;
  output.append("]");
  npulog(profile, cout << output << endl;)
}
void LoadBalancer::updateServerSessionsTable() {
  TestbedUtilities util;
  std::vector<std::string> headers =
    util.getPacketHeaders(rcvd_testbed_packet->getData());
  size_t urlLen = 0;
  uint32_t payloadPos = util.getHeaderLength(headers);
  std::vector<uint8_t> temp_vector;
  // We begin the payload by the URL in the same format as the Question in
  // DNS headers
  temp_vector.insert(temp_vector.begin(),
    rcvd_testbed_packet->setData().begin() + payloadPos,
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
  // Next, extract from the payload the prefix counts, prefix masks and lengths
  payloadPos += urlLen;
  // prefix counts - 32 bits
  uint32_t prefix_count = ntohl(*(static_cast<uint32_t*>(
    static_cast<void*>(rcvd_testbed_packet->setData().data() + payloadPos))));
  payloadPos += sizeof(uint32_t);
  std::vector<std::string> prefixes;
  for (size_t index = 0; index < prefix_count; index++) {
    struct in_addr *tempip;
    if (index != 0) {
      payloadPos += (sizeof(struct in_addr) + sizeof(uint32_t));
    }
    tempip = static_cast<struct in_addr*>(static_cast<void*>(
      rcvd_testbed_packet->setData().data() + payloadPos));
    std::string prefix_mask = inet_ntoa(*tempip);
    uint32_t prefix_len = ntohl(*(static_cast<uint32_t*>(
      static_cast<void*>(rcvd_testbed_packet->setData().data() + payloadPos
      + sizeof(struct in_addr)))));
    prefix_mask.append("/");
    prefix_mask.append(std::to_string(prefix_len));
    prefixes.push_back(prefix_mask);
  }
  updateReverseNAT(prefixes);
  payloadPos += (sizeof(struct in_addr) + sizeof(uint32_t));
  std::vector<std::string> baseIPs = util.getBaseIPs(prefixes);
  // Next extract the instance ids and loads
  uint32_t instances_count = ntohl(*(static_cast<uint32_t*>(
    static_cast<void*>(rcvd_testbed_packet->setData().data() + payloadPos))));
  payloadPos += sizeof(uint32_t);
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
    if (index != 0) {
      payloadPos += iid_load_len;
    }
    tempip = static_cast<struct in_addr*>(static_cast<void*>(
      rcvd_testbed_packet->setData().data() + payloadPos));
    std::string instance_id = inet_ntoa(*tempip);
    uint32_t instance_load = ntohl(*(static_cast<uint32_t*>(
      static_cast<void*>(rcvd_testbed_packet->setData().data() + payloadPos
      + sizeof(struct in_addr)))));
    // I want a server_sessions_table: node_id, instance_id, load
    // For the current implementation we are using the 0th baseIP as the
    // public IP address of the node
    instance_infotype instance_values(instance_id, instance_load,
      baseIPs.at(0));
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
void LoadBalancer::updateReverseNAT(const std::vector<std::string> &prefixes) {
  // For the current implementation, the 0th base IP will be the public URL
  TestbedUtilities util;
  std::string public_ip = util.getBaseIPs(prefixes).at(0);
  // for (std::map<std::string, std::string>::iterator it
  //  = reverse_nat_table.begin();
  //  it != reverse_nat_table.end(); ++it) {
  //  npulog(profile, cout << it->first << " -> " << it->second << endl;)
  // }
  for (std::string prefix : prefixes) {
    if (!reverse_nat_table.count(prefix)) {
      // cout << "reverse nat" << prefix << endl;
      // This is a new entry and needs to be inserted
      // Maintaining our records in the control plane
      reverse_nat_table.insert(std::pair<std::string, std::string>(
        prefix, public_ip));
      // Updating match-action tables in the forwarding plane
      pfp::cp::CommandParser parser;
      std::string insert_cmd ="insert_entry reverse_nat ";
      insert_cmd.append(prefix);
      insert_cmd.append(" perform_reverse_nat ");
      insert_cmd.append(public_ip);
      npulog(profile, cout << insert_cmd << endl;)
      auto cmd = parser.parse_line(insert_cmd);
      lbs->send_command(cmd);
    } else {
      // TODO(f_dewal): This is an old entry
      // Check if it needs to be updated
    }
  }
}
void LoadBalancer::updateForwardNAT(std::string client_ip,
  std::string public_ip, std::string server_ip) {
  pfp::cp::CommandParser parser;

  // Delete the entry
  if (forward_nat_table.count(client_ip)) {
    // We need to delete the table entries
    // Forwarding plane
    std::string del_cmd = "delete_entry forward_nat ";
    // entry handle
    del_cmd.append(std::to_string(forward_nat_table.find(client_ip)->second));
    npulog(profile, cout << del_cmd << endl;)
    auto cmd = parser.parse_line(del_cmd);
    lbs->send_command(cmd);

    // Control plane
    forward_nat_table.erase(client_ip);
  }

  // This is a new client DNS. Lets add it to both Control plane
  // and update the tables in forwarding plane as well
  // Updating match-action tables in the forwarding plane

  std::string insert_cmd ="insert_entry forward_nat ";
  insert_cmd.append(client_ip);
  insert_cmd.append(" ");
  insert_cmd.append(public_ip);
  insert_cmd.append(" perform_forward_nat ");
  insert_cmd.append(server_ip);
  npulog(profile, cout << insert_cmd << endl;)
  auto cmd = parser.parse_line(insert_cmd);
  std::shared_ptr<pfp::cp::CommandResult> handle = lbs->send_command(cmd);
  std::shared_ptr<pfp::cp::InsertResult> ir =
    std::dynamic_pointer_cast<pfp::cp::InsertResult>(handle);

  // Add handle also to the control plane table for future modifying
  size_t entry_handle = ir->handle;
  forward_nat_table.insert(std::pair<std::string, size_t>(
    client_ip, entry_handle));
}

  /*
  pfp::cp::CommandParser parser;
  std::string insert_cmd =
    "insert_entry ipv4_lpm 9.54.33.36/32 set_nhop 13.1.0.0 11";
  auto cmd = parser.parse_line(insert_cmd);
  std::shared_ptr<pfp::cp::CommandResult> handle = lbs->send_command(cmd);
  std::shared_ptr<pfp::cp::InsertResult> ir =
    std::dynamic_pointer_cast<pfp::cp::InsertResult>(handle);

  std::string mod_cmd = "modify_entry ipv4_lpm ";
  mod_cmd.append(std::to_string(ir->handle));
  mod_cmd.append(" set_nhop 14.1.6.3 12");
  cout << mod_cmd << endl;
  cmd = parser.parse_line(mod_cmd);
  cout << "Parsed" << endl;
  handle = lbs->send_command(cmd);
  std::shared_ptr<pfp::cp::ModifyResult> mr =
    std::dynamic_pointer_cast<pfp::cp::ModifyResult>(handle);

  std::string del_cmd = "delete_entry ipv4_lpm ";
  del_cmd.append(std::to_string(ir->handle));
  cmd = parser.parse_line(del_cmd);
  handle = lbs->send_command(cmd);
  */
