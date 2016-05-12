/*
 * MemoryUtilitites.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: Lemniscate Snickets
 */

#include "MemoryUtilities.h"
#include <vector>
#include <string>
#include "pfpsim/pfpsim.h"
// #define MemoryUtilities_DEBUG
MemoryUtilities::MemoryUtilities() {
  VirtualMemMaxSize = 0;
}
MemoryUtilities::memdecode
MemoryUtilities::decodevirtual(tlm_addr_ virtualaddr) {
  memdecode result;
  tlm_addr_ vaddr = virtualaddr;

  std::vector<tlm_addr_>::iterator it = memboundries.begin();
  std::vector<std::string>::iterator iter_memnames = memnames.begin();
  std::vector<std::string>::iterator iter_mempaths = mempaths.begin();

  while (it != memboundries.end()) {
    int currentMemBoundary = *it;
    if (vaddr < currentMemBoundary) {
      // this is correct module;
      result.physcialaddr = vaddr;
      result.memname = *iter_memnames;
      // result.mempath = "TEC_0."+result.memname;
      result.mempath = *iter_mempaths;
      return result;
    } else {
      vaddr = vaddr-currentMemBoundary;
      if (vaddr < 0) {
        std::cerr
              << "MemoryUtilities Address Decode -ve address Not possible"
              << endl;
        SC_REPORT_ERROR("MemoryUtilities-Address Decode", "-ve addr");
        sc_stop();
      }
    }
    if (it != memboundries.end()) {
      it++;  // go to next boundary address
      iter_memnames++;
      iter_mempaths++;
    } else {
      std::cerr
          << "MemoryUtilities Address Decode Virtual Addr is outside of range1"
          <<endl;
      SC_REPORT_ERROR("MemoryUtilities-Address Decode", "Out of range");
      sc_stop();
    }
  }
  std::cerr
        << "MemoryUtilities Address Decode Virtual Addr is outside of range2"
        << endl;
  SC_REPORT_ERROR("MemoryUtilities-Address Decode", "Out of range");
  sc_stop();
}

void MemoryUtilities::LoadMemoryConfig(std::string filename) {
  ifstream iFile;
  iFile.open(filename);
  if (iFile.is_open()) {
    char delimiter = ',';
    std::string line;
    std::vector<std::vector<std::string>> token_vector;
    while (getline(iFile, line)) { /* While there is still a line. */
      // Remove Whitespace in extracted line.
      line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
#ifdef MemoryUtilities_DEBUG
      cout << "Extracted Line: " << line << endl;
#endif
      // Split Line into strings based on delimiter
      std::vector<std::string> tokens = split(line, delimiter);
      token_vector.push_back(tokens);
    }
    for (std::vector<std::vector<std::string>>::iterator it
            = token_vector.begin();
         it != token_vector.end();
         ++it) {
      std::vector<std::string> vec = *it;
      if (it->at(0).find("mem") != std::string::npos) {
        memnames.push_back(it->at(0));
        mem_rd_latecy.push_back(std::stod(it->at(1)));
        mem_wr_latency.push_back(std::stod(it->at(2)));
        memboundries.push_back(stoi(it->at(3)));
        mempaths.push_back(it->at(4));

#ifdef MemoryUtilities_DEBUG
        for (std::vector<std::string>::iterator it2 = vec.begin();
            it2 != vec.end(); ++it2) {
          cout << "--" << *it2 << endl;
        }
#endif
      }
    }
#ifdef MemoryUtilities_DEBUG
    for (std::vector<std::string>::iterator it2 = memnames.begin();
        it2 != memnames.end(); ++it2) {
      cout << "MEM NAMES--" << *it2 << endl;
    }
#endif
    VirtualMemMaxSize = 0;
    for (std::vector<tlm_addr_>::iterator it2 = memboundries.begin();
         it2 != memboundries.end(); ++it2) {
      VirtualMemMaxSize = VirtualMemMaxSize + *it2;
#ifdef MemoryUtilities_DEBUG
      cout << "MEM SIZES--" << *it2 << endl;
#endif
    }
#ifdef MemoryUtilities_DEBUG
    for (std::vector<std::string>::iterator it2 = mempaths.begin();
        it2 != mempaths.end(); ++it2) {
      cout << "MEM PATHS--" << *it2 << endl;
    }
#endif
#ifdef MemoryUtilities_DEBUG
    cout << "Configured Max Virtual Memory Size: " << VirtualMemMaxSize
        << endl;
#endif

    iFile.close();
  } else {
    std::cerr << "Memory Configuration File Open Failed!" << endl;
    SC_REPORT_ERROR("MemoryUtilities", "Could not open Configuration File");
    sc_stop();
  }
}

std::vector<std::string>
MemoryUtilities::split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, elems);
  return elems;
}

std::vector<std::string>&
MemoryUtilities::split(const std::string &s, char delim,
    std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}
