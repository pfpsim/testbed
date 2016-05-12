/*
 * pdcomparison.h
 *
 *  Created on: Feb 8, 2016
 *      Author: Lemniscate Snickets / Shafigh
 */

#ifndef BEHAVIOURAL_COMMON_PDCOMPARISON_H_
#define BEHAVIOURAL_COMMON_PDCOMPARISON_H_

#include "PacketDescriptor.h"
#include <memory>

class pdcomparison {
 public:
  bool operator() (const std::shared_ptr<PacketDescriptor>& lhs,
                   const std::shared_ptr<PacketDescriptor>& rhs) const {
    return (lhs->id() > rhs->id());
  }
};

#endif  // BEHAVIOURAL_COMMON_PDCOMPARISON_H_
