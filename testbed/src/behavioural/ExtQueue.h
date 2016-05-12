#ifndef BEHAVIOURAL_EXTQUEUE_H_
#define BEHAVIOURAL_EXTQUEUE_H_

#include <string>
#include "../structural/ExtQueueSIM.h"
#include "TestbedRdI.h"
#include "TestbedWrI.h"

template<typename T>
class ExtQueue:
public TestbedRdI<T>,
public TestbedWrI<T>,
public ExtQueueSIM {
 public:
  /* CE Consturctor */
  ExtQueue(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile="");  // NOLINT

  /* User Implementation of the Virtual Functions in the Interface.h file */
  // Implementing get interfaces
  virtual bool nb_can_get(tlm::tlm_tag<T> *t = 0) const {
    return fifo_.nb_can_get(t);
  }
  virtual const sc_core::sc_event &ok_to_get(tlm::tlm_tag<T> *t = 0) const {
    return fifo_.ok_to_get(t);
  }
  virtual T read(tlm::tlm_tag<T> *t = 0 ) {return fifo_.get(t);}

  // Peek interface
  virtual T peek(tlm_tag<T> *t = 0) const {
    return fifo_.peek(t);
  }

  // Implementing put interfaces
  virtual bool nb_can_put(tlm_tag<T> *t = 0) const {
    return fifo_.nb_can_put(t);}
  virtual const sc_core::sc_event& ok_to_put(tlm_tag<T> *t = 0) const  {
    return fifo_.ok_to_put(t);
  }
  virtual void write(const T& t)  {
    fifo_.put(t);
  }

 private:
    tlm::tlm_fifo<T> fifo_;
};

/*
  Queue Consturctor
 */
template<typename T>
ExtQueue<T>::ExtQueue
  (sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):
    ExtQueueSIM(nm, parent, configfile),
    fifo_(static_cast<int>(GetParameter("FifoSize").get())) {
}

#endif  // BEHAVIOURAL_EXTQUEUE_H_
