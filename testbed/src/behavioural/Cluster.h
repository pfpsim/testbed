#ifndef BEHAVIOURAL_CLUSTER_H_
#define BEHAVIOURAL_CLUSTER_H_
#include <string>
#include <vector>
#include "../structural/ClusterSIM.h"

class Cluster: public ClusterSIM {
 public:
  SC_HAS_PROCESS(Cluster);
  /*Constructor*/
  explicit Cluster(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Cluster() = default;

 public:
  void init();
};

#endif  // BEHAVIOURAL_CLUSTER_H_
