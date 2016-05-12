#ifndef BEHAVIOURAL_QUEUEWRI_H_
#define BEHAVIOURAL_QUEUEWRI_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

template <typename T>
class QueueWrI : public sc_interface {
 public:
  /* User Logic - Virtual Functions for interface go here */
  virtual bool nb_can_put(tlm_tag<T> * = 0) const = 0;
  virtual const sc_core::sc_event& ok_to_put(tlm_tag<T> * = 0) const = 0;
  void put(const T& t);
  virtual void write(const T&) = 0;
};

template<typename T>
void QueueWrI<T>::put(const T& t) {
    sc_process_handle this_process = sc_get_current_process_handle();
    sc_object* current_scmodule = this_process.get_parent_object();
    pfp::core::PFPObject* module;
    module = dynamic_cast<pfp::core::PFPObject*>(current_scmodule);
    module->notify_data_written(t, sc_time_stamp().to_default_time_units());
    write(t);
}
#endif  // BEHAVIOURAL_QUEUEWRI_H_
