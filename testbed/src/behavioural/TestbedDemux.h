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

#ifndef BEHAVIOURAL_TESTBEDDEMUX_H_
#define BEHAVIOURAL_TESTBEDDEMUX_H_
#include <string>
#include <vector>
#include "../structural/TestbedDemuxSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"
#include "common/Packet.h"

class TestbedDemux: public TestbedDemuxSIM {
 public:
  SC_HAS_PROCESS(TestbedDemux);
  /*Constructor*/
  TestbedDemux(sc_module_name nm , int outPortSize , pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~TestbedDemux() = default;

 public:
  void init();

 private:
  void TestbedDemux_PortServiceThread();
  void TestbedDemuxThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  void analyzeMetrics();
  void processPacketStream();
  void reinsertPacket(std::shared_ptr<TestbedPacket> packet);

  PcapLogger *pcap_logger;
};

#endif  // BEHAVIOURAL_TESTBEDDEMUX_H_
