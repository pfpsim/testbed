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

TestbedMux::TestbedMux(sc_module_name nm , int inPortSize, pfp::core::PFPObject* parent, std::string configfile):TestbedMuxSIM(nm ,inPortSize,parent,configfile) {  // NOLINT
    pcapLogger = new PcapLogger("ingress_sctime.pcap");
    packetCount = 0;
    /*sc_spawn threads*/
    for (size_t index = 0; index < inPortSize; index++) {
      ThreadHandles.push_back(sc_spawn(sc_bind
        (&TestbedMux::TestbedMux_PortServiceThread, this, index)));
    }
    ThreadHandles.push_back(sc_spawn(sc_bind
      (&TestbedMux::TestbedMuxThread, this, 0)));
    ThreadHandles.push_back(sc_spawn(sc_bind
      (&TestbedMux::packetLoop_thread, this)));
}
void TestbedMux::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TestbedMux::TestbedMux_PortServiceThread(std::size_t port_num) {
  // Thread function to service input ports.
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(in[port_num]->get());
    muxLock.lock();
    packetCount++;
    pcapLogger->logPacket(packet->getData(), sc_time_stamp());

    packet->setIngressPort(port_num);

    // Just to make the Testbed work without the NPU model
    // In the NPU model, the egress port would be re-written
    // by the P4 application
    if (port_num %2 == 0) {
      // Typically this would from a client to a server
      // Hence, +1
      packet->setEgressPort(port_num+1);
    } else {
      packet->setEgressPort(port_num-1);
    }
    // Incrementing by one, because P4 port index has been configured to
    // start with 1 in the provided testbedRouting Table.txt
    packet->setEgressPort(packet->getEgressPort() + 1);
    // Servers are at one + to the clients

    incomingPackets.push(packet);
    npulog(profile, cout << packetCount << " packets sent to NPU" << endl;)
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
      npulog(profile, cout << "Stuck at NPU Ingress! This is bad! Logical"
      << " Time: " << sc_time_stamp() << endl;)
      gotStuck = true;
    }
    out->put(packet);
    if (gotStuck) {
      npulog(profile, cout << "Resumed packet flow to ingress at logical "
      << "time: " << sc_time_stamp() << endl;)
      gotStuck = false;
    }
  }
}
void TestbedMux::packetLoop_thread() {
  while (true) {
    std::shared_ptr<TestbedPacket> packet =
    std::dynamic_pointer_cast<TestbedPacket>(loop_in->get());
    muxLock.lock();
    pcapLogger->logPacket(packet->getData(), sc_time_stamp());
    incomingPackets.push(packet);
    muxLock.unlock();
  }
}
