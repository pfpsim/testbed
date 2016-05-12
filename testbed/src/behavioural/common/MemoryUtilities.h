/*
 * MemoryUtilities.h
 *
 *  Created on: Oct 28, 2015
 *      Author: Lemniscate Snickets
 *
 */

#ifndef MEMORYUTILITIES_H_
#define MEMORYUTILITIES_H_

#include <iostream>
#include <string>
#include <vector>

class MemoryUtilities {
 public:
  MemoryUtilities();
  ~MemoryUtilities() = default;
  typedef int tlm_addr_;
  void LoadMemoryConfig(std::string filename);
  std::vector<std::string> split(const std::string &s, char delim);
  std::vector<std::string>&
  split(const std::string &s, char delim, std::vector<std::string> &elems);

  struct memdecode{
    tlm_addr_ physcialaddr;
    std::string memname;
    std::string mempath;
  };

  memdecode decodevirtual(tlm_addr_ virtualaddr);
 protected:
  unsigned long long VirtualMemMaxSize;
  std::vector<tlm_addr_>memboundries;
  std::vector<std::string>memnames;
  std::vector<std::string>mempaths;
  std::vector<double>mem_rd_latecy;
  std::vector<double>mem_wr_latency;
};




#endif /* MEMORYUTILITIES_H_ */
