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
