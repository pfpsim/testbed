#ifndef BEHAVIOURAL_ROUTER_H_
#define BEHAVIOURAL_ROUTER_H_

#include <vector>
#include <string>
#include <utility>
#include "../structural/RouterSIM.h"
#include "common/RoutingPacket.h"
class Router: public RouterSIM {
 public:
  SC_HAS_PROCESS(Router);
  /*Constructor*/
  Router(sc_module_name nm , int ocn_rd_ifPortSize, int ocn_wr_ifPortSize , pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Router() = default;

 public:
  void init();

 private:
  void Router_PortServiceThread();
  void RouterThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  const std::string delimiter = ".";
  struct HierarchicalDestination {
    std::string sendto;
    std::string setdestinationto;
  };
  /**
   * Function that evaluate a  hierarichal destinations of the form Mod1.Mod2
   * @param  input Destination of the form Mod1.Mod2.Mod3
   * @return       struct {"Mod1","Mod2.Mod3"}
   */
  HierarchicalDestination EvaluateHierarchicalDestination(std::string input);

  json lookuptables;
  std::pair<int, bool> DetermineOutputPortNumber(std::string destination);
  std::pair<int, bool> SearchinConnectedNodes(std::string searchnode);
  std::pair<std::string, bool> SearchinNextNodes(std::string searchnode);
  MTQueue<std::shared_ptr<AbstractRoutingPacket>> ReceivedRoutingPackets;
  sc_event RouteTransactions;

  void ReadConfigfile(std::string filename);

  std::ofstream outlog;
  std::ofstream readlog;
  void LogPacket
  (std::ofstream& outputto, std::shared_ptr<AbstractRoutingPacket> packet);
  std::string configFile;
};
#endif  // BEHAVIOURAL_ROUTER_H_
