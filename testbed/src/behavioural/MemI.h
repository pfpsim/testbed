#ifndef BEHAVIOURAL_MEMI_H_
#define BEHAVIOURAL_MEMI_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

template <typename T>
class MemI : public sc_interface {
 public:
    /* User Logic - Virtual Functions for interface go here */
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay) = 0;
};
#endif  // BEHAVIOURAL_MEMI_H_
