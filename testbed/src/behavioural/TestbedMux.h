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

#ifndef BEHAVIOURAL_TESTBEDMUX_H_
#define BEHAVIOURAL_TESTBEDMUX_H_
#include <string>
#include <vector>
#include "../structural/TestbedMuxSIM.h"
#include "common/TestbedUtilities.h"
#include "common/TestbedPacket.h"
#include "common/PcapLogger.h"

class TestbedMux: public TestbedMuxSIM {
 public:
  SC_HAS_PROCESS(TestbedMux);
  /*Constructor*/
  TestbedMux(sc_module_name nm , int inPortSize , pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~TestbedMux() = default;

 public:
  void init();

 private:
  void TestbedMux_PortServiceThread(std::size_t port_num);
  void TestbedMuxThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  void BypassNPU(std::shared_ptr<pfp::core::TrType> inputS);
  void packetLoop_thread();

 private:
  MTQueue<std::shared_ptr<pfp::core::TrType> > incomingPackets;
  sc_mutex muxLock;
  PcapLogger *pcapLogger;
  uint64_t packetCount;
};

#endif  // BEHAVIOURAL_TESTBEDMUX_H_
