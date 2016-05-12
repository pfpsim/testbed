/*
 * TestApplication.cpp
 *
 *  Created on: Aug 15, 2014
 *      Author: kamil
 */
/**
 * @file TestApplication
 * This file contains a test application to verify the accuracy of the DICP profiling tool
 */

#include <iostream>
#include <string>
#include "ApplicationRegistry.hpp"
#include "P4.h"
#include "TestbedUtilities.h"

/**
 * Test application with various branches containing nonsense instructions of different sizes
 * @param counter	FOR PROFILING ONLY: holds the number of assembly instructions executed when the application is complete
 * @param np		PacketDescriptor to process
 * @param p			Packet to process
 * @param args		Additional arguments -- NOTE: These must be correctly recast by the application programmer
 * @return			Processed Packet
 */
Packet& P4Application(std::string parent_module_name, uint32_t &counter,
    PacketDescriptor &np, Packet &p, void *args = 0) {
  static auto p4 = P4::get(parent_module_name);  // parent_module_name);
  static auto ingress = p4->get_pipeline("ingress");
  static auto egress  = p4->get_pipeline("egress");
  auto p4_packet = np.header().get();
  p4->lock.read_lock();

  ingress->apply(p4_packet);


  // TODO(gordon) if( ! np.drop()){
  egress->apply(p4_packet);
  p4->lock.read_unlock();
  /*
  // We re-attach the full payload for the table application, incase it is
  // needed by the P4 program (to calculate a TCP checksum, for example)
  np.swapPayload(p.data().data(), p.data().size(), &payload, &length);
  bool ret = p4_do_table_application(np.header().get());
  // One processing is complete, we restore the original empty payload
  np.swapPayload(payload, length);
  */
  return p;
}

static int ts = register_application("P4Application", P4Application);
