#ifndef BEHAVIOURAL_CONTROLPLANEAGENTHALS_H_
#define BEHAVIOURAL_CONTROLPLANEAGENTHALS_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

class ControlPlaneAgentHalS : public sc_interface {
 public:
    /* User Logic - Virtual Functions for interface go here */
    typedef std::size_t TlmType;
    virtual void tlmwrite(int VirtualAddress, int data, TlmType size) = 0;
    virtual TlmType tlmread(int VirtualAddress) = 0;
    virtual TlmType tlmallocate(int BytestoAllocate) = 0;
};
#endif  // BEHAVIOURAL_CONTROLPLANEAGENTHALS_H_
