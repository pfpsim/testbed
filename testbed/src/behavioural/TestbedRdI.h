#ifndef BEHAVIOURAL_TESTBEDRDI_H_
#define BEHAVIOURAL_TESTBEDRDI_H_

#include "systemc.h"  // NOLINT
#include "tlm.h"  // NOLINT
using tlm::tlm_tag;

template <typename T>
class TestbedRdI : public sc_interface {
 public:
    /* User Logic - Virtual Functions for interface go here */
    virtual bool nb_can_get(tlm::tlm_tag<T> * = 0) const = 0;
    virtual const sc_core::sc_event &ok_to_get(tlm::tlm_tag<T> * = 0) const = 0;
    T get(tlm::tlm_tag<T> *t = 0);
    virtual T read(tlm::tlm_tag<T> * = 0) = 0;
    virtual T peek(tlm_tag<T> * = 0) const = 0;
};

template<typename T>
T TestbedRdI<T>::get(tlm::tlm_tag<T> *t) {
    sc_process_handle this_process = sc_get_current_process_handle();
    sc_object* current_scmodule = this_process.get_parent_object();
    pfp::core::PFPObject* module;
    module = dynamic_cast<pfp::core::PFPObject*>(current_scmodule);
    auto ret_val =  read(t);
    module->notify_data_read(ret_val, sc_time_stamp().to_default_time_units());
    return ret_val;
}


template <typename T>
inline bool nb_can_gets(const sc_vector<sc_port<TestbedRdI<T>>>& input_ports) {
    bool can_get = false;
    for (auto& port : input_ports) {
        can_get |= port->nb_can_get();
    }
    return can_get;
}

template <typename T>
inline sc_event_or_list
multiport_data_written_event
(const sc_vector<sc_port<TestbedRdI<T>>>& input_ports) {
    sc_event_or_list or_list;
    for (auto& port : input_ports) {
        or_list |= port->ok_to_get();
    }
    return or_list;
}
#endif  // BEHAVIOURAL_TESTBEDRDI_H_