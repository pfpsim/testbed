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

#ifndef BEHAVIOURAL_LOADBALANCER_H_
#define BEHAVIOURAL_LOADBALANCER_H_
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <tuple>
#include "../structural/LoadBalancerSIM.h"
#include "common/TestbedPacket.h"
#include "common/TestbedUtilities.h"

class LoadBalancer: public LoadBalancerSIM {  // NOLINT(whitespace/line_length)
 public:
  SC_HAS_PROCESS(LoadBalancer);
  /*Constructor*/
  explicit LoadBalancer(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~LoadBalancer() = default;

 public:
  void init();

 private:
  typedef std::tuple<std::string, size_t, std::string> instance_infotype;
  // typedef std::tuple<std::string, size_t> forward_nattype;

  void LoadBalancer_PortServiceThread();
  void LoadBalancerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  void outgoingPackets_thread();

  void invokeDNS_static();
  void invokeDNS_rr();
  void invokeDNS_shortestQ();
  void updateServerSessionsTable();
  void updateReverseNAT(const std::vector<std::string> &prefixes);
  void updateForwardNAT(std::string client_ip, std::string public_ip,
    std::string server_ip);

  std::shared_ptr<TestbedPacket> rcvd_testbed_packet;
  // node_id --> instance_id, instance_load
  std::multimap<std::string, instance_infotype>
    server_sessions_table;
  MTQueue<std::shared_ptr<pfp::core::TrType> > outgoing_packets;
  std::string getServerInstanceAddress();
  // server instance prefixes, public_ip
  std::map<std::string, std::string> reverse_nat_table;
  // client_ip, handle
  std::map<std::string, size_t> forward_nat_table;
  std::map<std::string, int> server_index;
  std::ofstream outlog;
};

#endif  // BEHAVIOURAL_LOADBALANCER_H_
