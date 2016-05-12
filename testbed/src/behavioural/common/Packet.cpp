/*
 * Packet.cpp
 *
 *  Created on: Jul 17, 2014
 *      Author: kamil
 */

#include "Packet.h"
#include <string>
#include "pfpsim/core/TrType.h"

Packet::Packet()
: Packet(0, 0, 4, packet_data_type()) {
}

Packet::Packet(std::size_t id, std::size_t context,
               std::size_t priority, const packet_data_type& packet_data)
: pfp::core::TrType(id), data_(packet_data), context_(context),
priority_(priority) {
  this->type_ = "packet";
}

/*Packet::Packet(const PacketDescriptor& pd, const packet_data_type& packet_data)
:  pfp::core::TrType(pd.id()), data_(packet_data), context_(pd.context())
{
}*/

std::size_t Packet::context_id() const {
  return context_;
}

Packet::packet_data_type& Packet::metadata() {
  return metadata_;
}

void Packet::metadata(const packet_data_type& packet_data) {
  metadata_ = packet_data;
}

Packet::packet_data_type& Packet::data() {
  return data_;
}

void Packet::data(const packet_data_type& packet_data) {
  data_ = packet_data;
}

void Packet::packet_priority(std::size_t priority) {
  priority_ = priority;
}

std::size_t Packet::packet_priority() const {
  return priority_;
}

std::string Packet::data_type() const {
  return "Packet";
}
std::size_t Packet::size() const {
  return data_.size();
}

void Packet::setEgressPort(std::size_t egressPort) {
  Packet::egressPort = egressPort;
}
std::size_t Packet::getEgressPort() {
  return egressPort;
}
