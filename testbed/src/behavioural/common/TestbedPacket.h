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

#ifndef BEHAVIOURAL_COMMON_TESTBEDPACKET_H_
#define BEHAVIOURAL_COMMON_TESTBEDPACKET_H_

#include <vector>
#include <string>
#include <cstdint>
#include "pfpsim/core/TrType.h"

class TestbedPacket : public pfp::core::TrType {
 public:
  TestbedPacket();

  TestbedPacket(const TestbedPacket& other) = default;

  virtual ~TestbedPacket() = default;

  void setData(const std::vector<uint8_t> &data);
  std::vector<uint8_t>& setData();
  const std::vector<uint8_t> getData();

  void setEgressPort(std::size_t  egressPort);
  std::size_t  getEgressPort();

  std::string data_type() const override;

 public:
  std::vector<uint8_t> _data;
  std::size_t egressPort;
};

#endif  // BEHAVIOURAL_COMMON_TESTBEDPACKET_H_
