/*
 * SystemCExtensions.h
 *
 *  Created on: Jan 10, 2016
 *      Author: Lemniscate Snickets
 */

#ifndef COMMON_SYSTEMCEXTENSIONS_H_
#define COMMON_SYSTEMCEXTENSIONS_H_

#include "../../QueueRdI.h"
#include "../../QueueWrI.h"
/**
 * Multi-port version of TLM's nb_can_get function
 * @param input_ports  Collection of ports to check
 * @return        True if any of the ports can get
 */
template <typename T>
inline bool nb_can_gets(const sc_vector<sc_port<QueueRdI<T>>>& input_ports) {
  bool can_get = false;
  for (auto& port : input_ports) {
    can_get |= port->nb_can_get();
  }
  return can_get;
}


/**
 * Multi-port version of TLM's ok_to_get function
 * @param input_ports  Collection of ports to check
 * @return        Or-list of the ok_to_get event from each port
 */
template <typename T>
inline sc_event_or_list
multiport_data_written_event(
  const sc_vector<sc_port<QueueRdI<T>>>& input_ports) {
  sc_event_or_list or_list;
  for (auto& port : input_ports) {
    or_list |= port->ok_to_get();
  }
  return or_list;
}



#endif /* COMMON_SYSTEMCEXTENSIONS_H_ */
