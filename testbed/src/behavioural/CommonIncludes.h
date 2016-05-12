/*
 * CommonIncludes.h
 *
 *  Created on: Jan 7, 2016
 *      Author: Lemniscate Snickets
 */

#ifndef BEHAVIOURAL_COMMONINCLUDES_H_
#define BEHAVIOURAL_COMMONINCLUDES_H_

/* STL + STD */
#include <exception>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <numeric>
#include <array>
#include <random>
#include <tuple>
#include <algorithm>
#include <deque>
#include <queue>
#include <list>
#include <limits>
#include <set>
#include <utility>
#include <iterator>

/* Interfaces / Services */
#include "behavioural/HalS.h"

/* Channel Data Structures */
#include "common/PacketDescriptor.h"
#include "common/Packet.h"
#include "common/TestbedPacket.h"
#include "common/IPC_MEM.h"

/* Application Layer */
#include "common/ApplicationRegistry.hpp"

/* Memory Utilities */
#include "common/MemoryUtilities.h"
#include "common/Memorymaps.h"
#include "common/MemoryDefines.h"

/* Testbed Utilities */
#include "common/TestbedUtilities.h"

/* Pcap Utilities */
#include "common/PcapLogger.h"
#include "common/PcapRepeater.h"

/* Random Crap*/
#include "common/pdcomparison.h"

/* TypeDefs */
typedef std::function<Packet&(Packet&, void*)> application_type;
#endif  // BEHAVIOURAL_COMMONINCLUDES_H_
