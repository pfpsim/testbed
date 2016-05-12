/*
 * Memorymaps.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: Lemniscate Snickets
 */

#include "Memorymaps.h"
#include <vector>
#include <string>
#include <map>
// #define MemoryUtilities_DEBUG

MemoryMaps::MemoryMaps() {
}

void MemoryMaps::LoadMemoryMAPConfig(std::string filename) {
  ifstream iFile;
  iFile.open(filename);
  if (iFile.is_open()) {
    char delimiter = ',';  // NOLINT
    std::string line;
    std::vector<std::vector<std::string>> token_vector;

    while (getline(iFile, line)) { /* While there is still a line. */
#ifdef MemoryUtilities_DEBUG
      cout << "Extracted Line: " << line << endl;
#endif
      // Split Line into strings based on delimiterp
      std::vector<std::string> tokens = mapsplit(line, delimiter);
      token_vector.push_back(tokens);
    }
    // Parse through tokens split using delimter
    for (
      std::vector<std::vector<std::string>>::iterator it = token_vector.begin();
      it != token_vector.end();
      ++it) {
      std::vector<std::string> vec = *it;
      if (it->at(0).find("mem") != std::string::npos) {
#ifdef MemoryUtilities_DEBUG
        for (std::vector<std::string>::iterator it2 = vec.begin();
             it2 != vec.end(); ++it2) {
          cout << "--" << *it2 << endl;
        }
#endif
        memnames_map.push_back(it->at(0));
        mem_rd_latecy_map.push_back(std::stod(it->at(1)));
        mem_wr_latency_map.push_back(std::stod(it->at(2)));
        memboundries_map.push_back(std::stoi(it->at(3)));
        mempaths_map.push_back(it->at(4));
        char delimiterteus = ' ';
        std::vector<std::string> teutokens = mapsplit(it->at(5), delimiterteus);
        for (std::vector<std::string>::iterator it67 = teutokens.begin();
            it67 != teutokens.end(); ++it67) {
          teulist_map.insert(std::make_pair(*it67, it->at(0)));
        }
      }
    }
#ifdef MemoryUtilities_DEBUG
    for (std::vector<std::string>::iterator it2 = memnames_map.begin();
         it2 != memnames_map.end(); ++it2) {
      cout << "MEM NAMES--" << *it2 << endl;
    }
    for (std::vector<std::string>::iterator it2 = mempaths_map.begin();
         it2 != mempaths_map.end(); ++it2) {
      cout << "MEM PATHS--" << *it2 << endl;
    }
#endif

    iFile.close();
    npulog(
      std::cout << "..::MemoryMAP-Loaded Memory Config::.."
                << std::endl << std::endl;)
  } else {
    std::cerr << "Memory Configuration File Open Failed!" << endl;
    SC_REPORT_ERROR("MemoryUtilities", "Could not open Configuration File");
    sc_stop();
  }
}

std::vector<std::string> MemoryMaps::getmempaths_map() {
  return mempaths_map;
}

std::vector<std::string>MemoryMaps::mapsplit(const std::string &s, char delim) {
  std::vector<std::string> elems;
  mapsplit(s, delim, elems);
  return elems;
}

std::vector<std::string>& MemoryMaps::mapsplit(const std::string &s, char delim,
    std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

// template<typename T>
// T MemoryMaps::SafeStringtoNum(std::string input) {
//   T result;
//   try {
//      result = std::stoi(input);
//   }
//   catch (std::invalid_argument&)
//   {
//      return false;
//   }
//   catch (std::out_of_range&)
//   {
//      return false;
//   }
// }
