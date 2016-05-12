#ifndef BEHAVIOURAL_COMMON_PACKET_H_
#define BEHAVIOURAL_COMMON_PACKET_H_

#include <vector>
#include <cstdint>
#include <string>
#include "pfpsim/core/TrType.h"

class Packet : public pfp::core::TrType {
 public:
  typedef std::vector<uint8_t> packet_data_type;

 public:
  Packet();

  /**
   * Construct a Packet
   * @param id      Packet ID
   * @param packet_data  Packet data
   */
  Packet(std::size_t id, std::size_t context,
        std::size_t priority, const packet_data_type& packet_data);
  /**
   * Construct a Packet
   * @param pd      PacketDescriptor from which to extract information
   * @param packet_data  Packet data
   */
  // Packet(const PacketDescriptor& pd, const packet_data_type& packet_data);
  /**
   * Copy-construct a Packet
   * @param other  Packet to copy
   */
  Packet(const Packet& other) = default;
  /**
   * Default destructor
   */
  virtual ~Packet() = default;

  /**
   * Get the Context id
   * @return  Context id
   */
  std::size_t context_id() const;
  /**
   * Set the Context id
   * @param context  Context ID
   */
  void context_id(std::size_t context);

  /**
   * Get the Packet data
   * @return  Packet data
   */
  packet_data_type& data();
  /**
   * Set the Packet data
   * @param packet_data  Packet data
   */
  void data(const packet_data_type& packet_data);

  /**
   * Get the Packet metadata
   * @return  Packet metadata
   */
  packet_data_type& metadata();
  /**
   * Set the Packet metadata
   * @param packet_data  Packet metadata
   */
  void metadata(const packet_data_type& packet_data);

  /**
   * Get the size of the Packet data
   * @return  Size of Packet data
   */
  std::size_t size() const;

  /**
   * Print the Packet as a JSON string to the specified output stream
   * @param out  Output stream
   * @param v    Packet to print
   * @return    Reference to the output stream
   */

  void packet_priority(std::size_t priority);
  /**
   * Set the Priority of the associated Packet
   * @param priority Packet priority
   */
  std::size_t packet_priority() const;

  std::string data_type() const override;

  void setEgressPort(std::size_t  egressPort);
  std::size_t  getEgressPort();


 public:
  std::string target_mem_dest;  // routing path
  std::string dest_target_memname_;  // name
  typedef int tlm_addr_;
  tlm_addr_ payload_vaddr;
  tlm_addr_ payload_paddr;


 private:
  packet_data_type data_;  /*!< Packet data */
  packet_data_type metadata_;
  std::size_t context_;  /*!< Context ID */
  std::size_t priority_;  /*!< Packet Priority */
  std::size_t size_;
  std::string type_;
  std::size_t egressPort;
};

#endif  // BEHAVIOURAL_COMMON_PACKET_H_
