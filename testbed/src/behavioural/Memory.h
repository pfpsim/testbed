#ifndef BEHAVIOURAL_MEMORY_H_
#define BEHAVIOURAL_MEMORY_H_

#include <string>
#include "../structural/MemorySIM.h"
#include "MemI.h"

#include "common/Memorymaps.h"
// #define debug_tlm_mem_transaction 1
template<typename T>
class Memory:
public MemI<T>,
public MemorySIM,
public MemoryMaps {
 public:
  /* CE Consturctor */
  Memory
  (sc_module_name nm,
   pfp::core::PFPObject* parent = 0, std::string configfile = "");
  /* User Implementation of the Virtual Functions in the Interface.h file */
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();
    // check address range
    if (adr >= static_cast<sc_dt::uint64>(SIZE) ||
        byt != 0 || len > 4 || wid < len) {
      std::cerr << "tlm_memory " << modulename
            << " Target does not support given mem payload transaction addr:"
            <<adr << endl;
      std::cerr << "tlm_memory - You probably have an out of range address"
            <<endl;
      SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");  // NOLINT(whitespace/line_length)
      sc_stop();
    }
    // Read or Write depending upon command.
    if ( cmd == tlm::TLM_READ_COMMAND ) {
      memcpy(ptr, &mem[adr], len);
    } else if ( cmd == tlm::TLM_WRITE_COMMAND ) {
      memcpy(&mem[adr], ptr, len);
    }
    // response status to indicate successful completion
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }

 private:
  std::string modulename;
  int* mem;
  uint64_t SIZE;
  sc_time RD_LATENCY;
  sc_time WR_LATENCY;
};

/*
  Memory Consturctor
 */
template<typename T>
Memory<T>::Memory
  (sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile)
  : MemorySIM(nm, parent, configfile) {
  // Search in MemoryMap for itself and get its own parameters
  LoadMemoryMAPConfig(CONFIGROOT+"memorymap_48_12edrams.cfg");
  sc_object* parentmod = this->get_parent_object();
  std::string parentname = parentmod->basename();
  modulename = parentname;
  int count = memnames_map.size();
  for (int i = 0; i < count; i++) {
    if (memnames_map.at(i).find(modulename) != std::string::npos) {
      sc_time rd_lat(mem_rd_latecy_map.at(i), SC_NS);
      sc_time wr_lat(mem_wr_latency_map.at(i), SC_NS);
      int mem_size = memboundries_map.at(i);
      RD_LATENCY = rd_lat;
      WR_LATENCY = wr_lat;
      SIZE = mem_size;
    }
  }
  mem = new int[SIZE];  // allocate the mem block on host mem in heap
  // Initialize memory with random data
  for (int i = 0; i < SIZE; i++) {
    mem[i] = 0xAA000000;
  }
}

#endif  // BEHAVIOURAL_MEMORY_H_
