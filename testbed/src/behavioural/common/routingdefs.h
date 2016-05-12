#ifndef BEHAVIOURAL_COMMON_ROUTINGDEFS_H_
#define BEHAVIOURAL_COMMON_ROUTINGDEFS_H_

#include <string>

// TLMVAR
#define ApplicationLayerName "applayer"
#define ControlPlaneAgentName "cpagent"

namespace {  // NOLINT(build/namespaces)

  constexpr auto PathtoMemoryManager = "cluster_0.memory_manager";
  constexpr auto cluster_prefix = "cluster_";
  constexpr auto core_prefix = "core_";
}

#endif // BEHAVIOURAL_COMMON_ROUTINGDEFS_H_
