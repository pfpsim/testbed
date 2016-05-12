/*
 * IPC_MEM.h
 *
 *  Created on: Jul 17, 2015
 *      Author: Lemniscate Snickets
 */
/**
 * @class IPC_MEM
 * This class represents inter module/process communication for mem mangment data and basic associated identifiers
 */
#ifndef BEHAVIOURAL_COMMON_IPC_MEM_H_
#define BEHAVIOURAL_COMMON_IPC_MEM_H_


#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include "pfpsim/pfpsim.h"
class IPC_MEM: public pfp::core::TrType {
 public:
  /**
   * @typedef Type of IPC_MEM data
   */
  typedef std::string packet_data_type;
  typedef std::size_t tlm_mem_type;

  IPC_MEM();
  /**
   * Construct a IPC_MEM
   * @param id      Packet ID
   * @param packet_data  Packet data
   */
  IPC_MEM(std::size_t id, tlm_mem_type tlm_address, std::string RequestType);
  /**
   * Construct a IPC_MEM
   * @param pd      PacketDescriptor from which to extract information
   * @param packet_data  Packet data
   */
  IPC_MEM(const IPC_MEM& pd, const packet_data_type& packet_data);
  /**
   * Copy-construct a Packet
   * @param other  Packet to copy
   */
  IPC_MEM(const IPC_MEM& other) = default;
  /**
   * Default destructor
   */
  ~IPC_MEM() = default;

  packet_data_type& data();
  void data(const packet_data_type& packet_data);

  std::size_t size() const;
  void size(std::size_t size);

  std::string data_type() const;

 public:
  packet_data_type data_;        //!< Data for debugging
  std::size_t id_;               //!< request id
  std::size_t size_;             //!< Size of IPC_MEM data_

  //! address it needs to read to
  //! address it needs to write to
  //! address returned from allocation
  tlm_mem_type tlm_address;

  tlm_mem_type bytes_to_allocate;    //!< size for allocation
  std::string RequestType;           //!< READ, WRITE, ALLOCATE
  //! path to  target Memory Module, that the address belongs to
  std::string target_mem_mod;
  std::string target_mem_name;
  //! tlm_data return for read from tlm_address
  //! tlm_data that it needs to write to tlm_address
  tlm_mem_type tlm_data;
  std::string table_name;
};
#endif  // BEHAVIOURAL_COMMON_IPC_MEM_H_
