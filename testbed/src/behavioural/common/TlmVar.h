/*
 * TlmVar.h
 *
 *  Created on: June 21, 2015
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMON_TLMVAR_H_
#define BEHAVIOURAL_COMMON_TLMVAR_H_

#include <string>

class tlmvar {
 public:
  tlmvar();
  virtual ~tlmvar() = default;

  // TLM Memory related functions refactor for new tries
  std::size_t allocate_mem(int size_of_data);  // get address
  void allocate(std::size_t data_to_allocate, int addr);  // write function
  std::size_t read_mem(int addr, std::size_t val_compare = 0);  // read function

  // Wrapper for allocate to not break legacy
  void write_mem(std::size_t data_to_allocate, int addr);

  unsigned long wordsizetoBytes(std::size_t wordsize);  // NOLINT

  typedef int tlm_addr_;
  struct memdecode{
    tlm_addr_ physcialaddr;
    std::string memname;
  };
  static std::string module_name_;
};
#endif  // BEHAVIOURAL_COMMON_TLMVAR_H_
