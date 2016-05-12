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
std::string TestbedPacket::data_type() const {
  return "TestbedPacket";
}
std::vector<uint8_t>& TestbedPacket::setData() {
  return _data;
}
