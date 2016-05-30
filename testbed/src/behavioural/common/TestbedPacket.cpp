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

#include "TestbedPacket.h"
#include <string>
#include <vector>

TestbedPacket::TestbedPacket() {
}

void TestbedPacket::setData(const std::vector<uint8_t> &data) {
  _data.insert(_data.begin(), data.begin(), data.end());
}
const std::vector<uint8_t> TestbedPacket::getData() {
  return _data;
}


void TestbedPacket::setEgressPort(std::size_t egressPort) {
  TestbedPacket::egressPort = egressPort;
}
std::size_t TestbedPacket::getEgressPort() {
  return egressPort;
}

void TestbedPacket::setIngressPort(std::size_t ingressPort) {
  TestbedPacket::ingressPort = ingressPort;
}
std::size_t TestbedPacket::getIngressPort() {
  return ingressPort;
}

std::string TestbedPacket::data_type() const {
  return "TestbedPacket";
}
std::vector<uint8_t>& TestbedPacket::setData() {
  return _data;
}
