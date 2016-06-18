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

#include "./TestbedMux.h"
#include <string>
#include <vector>
#include <chrono>


TestbedMux::TestbedMux(sc_module_name nm , int inPortSize, pfp::core::PFPObject* parent, std::string configfile):TestbedMuxSIM(nm ,inPortSize,parent,configfile) {  // NOLINT
    pcap_logger = new PcapLogger("ingress_sctime.pcap");
    packetCount = 0;
    /*sc_spawn threads*/
    for (size_t index = 0; index < inPortSize; index++) {
      ThreadHandles.push_back(sc_spawn(sc_bind
        (&TestbedMux::TestbedMux_PortServiceThread, this, index)));
    }
    ThreadHandles.push_back(sc_spawn(sc_bind
      (&TestbedMux::TestbedMuxThread, this, 0)));
}
void TestbedMux::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TestbedMux::TestbedMux_PortServiceThread(std::size_t port_num) {
  // Thread function to service input ports.
  // Utilites for converting Testbed Packet to Npu Packet
  auto seed =
    std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::mt19937 rng(seed);
  int contexts = SimulationParameters["contexts"].get();
  int isolation_groups = SimulationParameters["isolation_groups"].get();
  std::uniform_int_distribution<std::size_t> uid_contexts(0, contexts - 1);
  std::uniform_int_distribution<std::size_t>
    uid_isolation_groups(0, isolation_groups - 1);

  while (true) {
    auto received_packet = in[port_num]->get();
    TestbedUtilities util;

    muxLock.lock();
    std::shared_ptr<TestbedPacket> testbed_packet =
      std::dynamic_pointer_cast<TestbedPacket>(received_packet);
    packetCount++;

    std::string outputstr;
    outputstr.append("Mux received packet (");
    outputstr.append(std::to_string(packetCount));
    outputstr.append(") from ");
    outputstr.append(std::to_string(port_num));
    outputstr.append(" port. ");
    outputstr.append("Headers are: ");
    std::vector<std::string> headers =
      util.getPacketHeaders(testbed_packet->getData());

    for (std::string hdr : headers) {
      outputstr.append(hdr);
      outputstr.append(" ");
    }
    npulog(profile, cout << Blue << outputstr << txtrst << endl;)


    pcap_logger->logPacket(testbed_packet->getData(), sc_time_stamp());
    if (headers.size() == 1) {
      util.getPacketHeaders(testbed_packet->getData());
      util.dissectPacket(testbed_packet->getData(), headers);
      assert(!"Incorrect config");
    }

    testbed_packet->setIngressPort(port_num);

    // Converting testbed packet to NPU packet format

    std::size_t context         = 1;  // uid_contexts(rng);
    std::size_t isolation_group = uid_isolation_groups(rng);
    Packet npu_packet(packetCount, context, -1, testbed_packet->getData());
    auto npu_packet_tuple = std::make_tuple
                        (std::make_shared<Packet>(npu_packet),
                        context, isolation_group);
    auto input_stimulus_packet = std::make_shared<InputStimulus>
      (packetCount, npu_packet_tuple);

    incomingPackets.push(input_stimulus_packet);
    muxLock.unlock();
  }
}
void TestbedMux::TestbedMuxThread(std::size_t thread_id) {
  // Thread function for module functionalty
  bool gotStuck = false;
  while (true) {
    std::shared_ptr<pfp::core::TrType> packet = incomingPackets.pop();
    timeval cpTime;
    gettimeofday(&cpTime, NULL);
    // BypassPacket bp(0, sc_time_stamp(), cpTime);
    // bypass->put(&bp);
    if (!out->nb_can_put()) {
      assert(!"Mux got blocked at NPU ingress! Increase fifo?");
      gotStuck = true;
    }
    out->put(packet);
    // Trying to implement network delays for every packet send
    // wait(.5, SC_MS);
    if (gotStuck) {
      gotStuck = false;
    }
  }
}
