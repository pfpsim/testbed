/*
 * PacketDescriptor.h
 *
 *  Created on: Jul 17, 2014
 *      Author: kamil
 */
#ifndef BEHAVIOURAL_PACKETDESCRIPTOR_H_
#define BEHAVIOURAL_PACKETDESCRIPTOR_H_


#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include "pfpsim/pfpsim.h"

#include "Packet.h"
#include "P4.h"

// Forward declaration
namespace bm {
class Packet;
}

class PacketDescriptor: public pfp::core::PacketBase {
 public:
  typedef std::vector<char> raw_header_t;
  typedef std::shared_ptr<bm::Packet> header_t;

 public:
  /**
   * Construct a PacketDescriptor
   */
  PacketDescriptor();
  /**
   * Construct a PacketDescriptor
   * @param id                     ID of the Packet described by this PacketDescriptor
   * @param packet_context         Context [0, 1023]
   * @param packet_isolation_group Isolation group [0, 3]
   * @param raw_header              Raw data of the packet header
   */
  PacketDescriptor(std::string parent_module_name, std::size_t id,
      std::size_t packet_context, std::size_t packet_isolation_group,
      raw_header_t raw_header, std::size_t packet_rcos = 0,
      std::size_t priority = 0);
  /**
   * Copy-construct a PacketDescriptor
   * @param other  PacketDescriptor to copy
   */
  PacketDescriptor(const PacketDescriptor& other) = default;
  /**
   * Move-construct a PacketDescriptor
   *
   */
  PacketDescriptor(PacketDescriptor &&other) = default;

  /**
   * Default destructor
   */
  virtual ~PacketDescriptor() = default;

  /**
   * Copy-assignment operator
   * @param other  PacketDescriptor to copy
   * @return    Reference to this PacketDescriptor
   */
  PacketDescriptor& operator=(const PacketDescriptor& other) = default;
  /**
   * Move-assignment operator
   * @param other  PacketDescriptor to move
   * @return    Reference to this PacketDescriptor
   */
  PacketDescriptor& operator=(PacketDescriptor &&other) = default;

  /**
   * Test for equality against another PacketDescriptor
   * @param other  PacketDescriptor against which to test
   * @return    True if this PacketDescriptor is equal to the other
   */
  bool operator==(const PacketDescriptor& other) const;


  void swapPayload(uint8_t * payload, size_t length,
                    uint8_t ** old_payload = NULL, size_t * old_length = NULL);

 public:
  /**
   * Parse the contents of the stored raw header data into a structured internal representation
   */
  int parse(std::string parent_module_name);

  /**
   * Deparse the internal P4 header structure and return the result
   */
  raw_header_t deparse(std::string parent_module_name);

  /**
   * Set the Priority of the associated Packet
   * @param priority Packet priority
   */
  void packet_priority(std::size_t priority);

  std::size_t packet_priority() const;
  /**
   * Get the Packet context
   * @return  Packet context
   */
  std::size_t context() const;
  /**
   * Set the Packet context
   * @param packet_context  Packet context
   * @return          True if the context was changed
   */
  void context(std::size_t packet_context);

  std::size_t resource_class_of_service() const;

  void resource_class_of_service(std::size_t packet_rcos);
  /**
   * Get the Packet drop status
   * @return  Drop status
   */
  bool drop() const;
  /**
   * Set the Packet drop status
   * @param packet_drop  Drop status
   * @return        True if the drop status was changed
   */
  void drop(const bool &packet_drop);
  /**
   * Get the header
   * @return  Packet header
   */
  header_t& header();
  /**
   * Set the header
   * @param packet_header  Packet header
   * @return        True if the header was changed
   */
  void header(const header_t& packet_header);
  /**
   * Get the isolation group
   * @return  Isolation group
   */
  std::size_t isolation_group() const;
  /**
   * Set the isolation group
   * @param packet_isolation_group  Isolation group
   * @return              True if the isolation group was changed
   */
  void isolation_group(std::size_t packet_isolation_group);

  std::string data_type() const override;

  bool debuggable() const override;

 public:
  typedef int tlm_addr_;
  std::string payload_target_mem_;
  tlm_addr_ payload_vaddr;
  tlm_addr_ payload_paddr;
  std::string payload_target_memname_;

 private:
  std::size_t packet_context_;
  std::size_t packet_ig_;
  std::size_t packet_rcos_;

  header_t packet_header_;

  std::size_t packet_priority_;
};

#endif  // BEHAVIOURAL_PACKETDESCRIPTOR_H_
