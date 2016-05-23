/*
 * simple-npu: Example NPU simulation model using the PFPSim Framework
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

#ifndef BEHAVIOURAL_UDPCLIENT_H_
#define BEHAVIOURAL_UDPCLIENT_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/UDPClientSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class UDPClient: public UDPClientSIM {
 public:
  SC_HAS_PROCESS(UDPClient);
  /*Constructor*/
  explicit UDPClient(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~UDPClient() = default;

 public:
  void init();

 private:
  void UDPClient_PortServiceThread();
  void UDPClientThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  // Port service methods
  void outgoingPackets_thread();
  void validatePacketDestination_thread();

  // Administrative methods
  void populateLocalMap();
  void addClientInstances();
  void activateClientInstance_thread();
  void scheduler_thread();

  // Behavioral methods
  void requestServerInstance(std::string clientID);
  void establishConnection(std::string clientID = NULL, std::string serverID = NULL);
  void requestFile();
  void registerFile();
  void processFile();
  void teardownConnection();

 private:
  sc_event activate_client_instance_event;
  std::map<std::string, std::string> localMap;
  std::shared_ptr<PcapLogger> pcapLogger;
  ClientConfigStruct ncs;
  MTQueue<std::shared_ptr<pfp::core::TrType> > outgoingPackets;
  std::map<std::string, struct ConnectionDetails> clientDetails;
  std::shared_ptr<TestbedPacket> receivedPacket;
};

#endif  // BEHAVIOURAL_UDPCLIENT_H_
