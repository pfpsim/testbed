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

#include "./TestbedDemux.h"
#include <string>

TestbedDemux::TestbedDemux(sc_module_name nm , int outPortSize, pfp::core::PFPObject* parent, std::string configfile):TestbedDemuxSIM(nm ,outPortSize,parent,configfile) {  // NOLINT
  pcapLogger = new PcapLogger("egress_sctime.pcap");
    /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TestbedDemux::processPacketStream, this)));
  ThreadHandles.push_back(sc_spawn(sc_bind
    (&TestbedDemux::analyzeMetrics, this)));
}
void TestbedDemux::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TestbedDemux::TestbedDemux_PortServiceThread() {
  // Thread function to service input ports.
}
void TestbedDemux::TestbedDemuxThread(std::size_t thread_id) {
  // Thread function for module functionalty
}
void TestbedDemux::processPacketStream() {
  while (true) {
    auto incoming_packet = in->get();
    if (std::shared_ptr<TestbedPacket> testbed_packet =
      std::dynamic_pointer_cast<TestbedPacket>(incoming_packet)) {
      size_t port_number = testbed_packet->getEgressPort() - 1;
      if (port_number >= out.size()) {
        npulog(profile, cout << "FATAL: Going for port number: "
        << port_number << endl;)
        // reinsertPacket(outgoing);
        assert(false);
      } else {
        npulog(profile, cout << "Going for port number: " << port_number
        << endl;)
        out[port_number]->put(testbed_packet);
        pcapLogger->logPacket(testbed_packet->getData(), sc_time_stamp());
      }
    } else if (std::shared_ptr<Packet> npu_packet =
      std::dynamic_pointer_cast<Packet>(incoming_packet)) {
      size_t port_number = npu_packet->getEgressPort() - 1;
      std::shared_ptr<TestbedPacket> testbed_packet =
        std::make_shared<TestbedPacket>();
      testbed_packet->setData().insert(testbed_packet->setData().begin(),
        npu_packet->data().begin(), npu_packet->data().end());
      testbed_packet->setEgressPort(port_number);
      if (port_number >= out.size()) {
        npulog(profile, cout << "FATAL: Going for port number: "
        << port_number << endl;)
        // reinsertPacket(outgoing);
        assert(false);
      } else {
        npulog(profile, cout << "Going for port number: " << port_number
        << endl;)
        out[port_number]->put(testbed_packet);
        pcapLogger->logPacket(testbed_packet->getData(), sc_time_stamp());
      }
    }
  }
}
void TestbedDemux::analyzeMetrics() {
  // while (true) {
  //  std::shared_ptr<pfp::core::TrType> bp = bypass->get();
  // delete(bp);
  // }
}
void TestbedDemux::reinsertPacket(std::shared_ptr<TestbedPacket> packet) {
  loop_out->put(packet);
}
