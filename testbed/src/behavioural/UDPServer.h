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

#ifndef BEHAVIOURAL_UDPSERVER_H_
#define BEHAVIOURAL_UDPSERVER_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/UDPServerSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class UDPServer: public UDPServerSIM {
 public:
  SC_HAS_PROCESS(UDPServer);
  /*Constructor*/
  explicit UDPServer(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~UDPServer() = default;

 public:
  void init();

 private:
  void UDPServer_PortServiceThread();
  void UDPServerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  // Administrative methods
  void populateLocalMap();
  void validatePacketSource_thread();
  void outgoingPackets_thread();
  void datarateManager_thread();
  std::string serverSessionsManager();
  void assignServer();

  // Behavioral methods
  void establishConnection();
  void registerFile();
  void processFile();
  void teardownConnection();

 private:
  std::shared_ptr<TestbedPacket> received_packet;
  std::map<std::string, std::string> local_map;
  std::shared_ptr<PcapLogger> pcap_logger;
  ServerConfigStruct ncs;
  MTQueue<std::shared_ptr<pfp::core::TrType> > outgoing_packets;
  std::map<std::string, struct ConnectionDetails> client_instances;
  std::map<std::string, size_t> server_sessions;
};

#endif  // BEHAVIOURAL_UDPSERVER_H_
