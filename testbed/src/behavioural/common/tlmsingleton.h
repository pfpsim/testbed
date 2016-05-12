/*
 * tlmsingleton.h
 *
 *  Created on: Oct 26, 2015
 *      Author: Lemniscate Snickets
 */
#include "TlmVar.h"
#ifndef BEHAVIOURAL_COMMON_TLMSINGLETON_H_
#define BEHAVIOURAL_COMMON_TLMSINGLETON_H_


class tlmsingelton {
 public:
  static tlmsingelton& getInstance() {
    static tlmsingelton instance;  // Guaranteed to be destroyed.
                                   // Instantiated on first use.
    return instance;
  }
  tlmvar* tlmvarptr;
 private:
  tlmsingelton() {          // Constructor? (the {} brackets) are needed here.
#if debug_singleton_tree_ref
    std::cout << "Singleton object instantiated" << std::endl;
#endif
  }
  tlmsingelton(tlmsingelton const&)     = delete;  // delete copies
  void operator = (tlmsingelton const&)  = delete;  // delete instantiation
};


#endif  // BEHAVIOURAL_COMMON_TLMSINGLETON_H_
