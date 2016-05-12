#ifndef BEHAVIOURAL_APPLICATIONLAYER_H_
#define BEHAVIOURAL_APPLICATIONLAYER_H_
#include <string>
#include <vector>
#include "../structural/ApplicationLayerSIM.h"

class ApplicationLayer: public ApplicationLayerSIM {
 public:
  SC_HAS_PROCESS(ApplicationLayer);
  /*Constructor*/
  explicit ApplicationLayer(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~ApplicationLayer() = default;

 public:
  void init();
  void do_processing(std::size_t thread_id,
                     PacketDescriptor& pd, Packet& payload);
 private:
  void ApplicationLayerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;
};

#endif  // BEHAVIOURAL_APPLICATIONLAYER_H_
