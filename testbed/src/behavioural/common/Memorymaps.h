/*
 * Memorymaps.h
 *
 *  Created on: Nov 5, 2015
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMON_MEMORYMAPS_H_
#define BEHAVIOURAL_COMMON_MEMORYMAPS_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "pfpsim/pfpsim.h"

class MemoryMaps {
 public:
  MemoryMaps();
  ~MemoryMaps() = default;
  typedef int tlm_addr_;
  void LoadMemoryMAPConfig(std::string filename);
  std::vector<std::string> mapsplit(const std::string &s, char delim);
  std::vector<std::string>&
  mapsplit(const std::string &s, char delim, std::vector<std::string> &elems);

  struct memmap{
    std::string memname;
    std::string mempath;
  };

  memmap gettargetmem(std::string teu_name);

  std::vector<std::string> getmempaths_map();


 protected:
  std::vector<std::string>memnames_map;
  std::vector<std::string>mempaths_map;
  std::map<std::string, std::string> teulist_map;

  std::vector<tlm_addr_>memboundries_map;
  std::vector<double>mem_rd_latecy_map;
  std::vector<double>mem_wr_latency_map;

  // template<typename T>
  // T SafeStringtoNum(std::string input);
};

#endif // BEHAVIOURAL_COMMON_MEMORYMAPS_H_
