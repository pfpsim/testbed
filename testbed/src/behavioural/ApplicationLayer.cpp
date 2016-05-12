#include "./ApplicationLayer.h"
#include <string>
#include <vector>
#include "common/ApplicationRegistry.hpp"

ApplicationLayer::ApplicationLayer(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ApplicationLayerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
  int teu_threads = GetParameter("core_threads").get();
  for (std::vector<sc_process_handle>::size_type i = 0; i < teu_threads; i++) {
    ThreadHandles.push_back(
      sc_spawn(sc_bind(&ApplicationLayer::ApplicationLayerThread, this, i)));
  }
}

void ApplicationLayer::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}

void ApplicationLayer::ApplicationLayerThread(std::size_t thread_id) {
  while (1) {
      std::shared_ptr<PacketDescriptor> pd;
      std::shared_ptr<Packet> payload;
      // TODO(Lemniscate): Add status checking on return from HAL
      halport->GetJobfromSchedular(thread_id, &pd, &payload);
      if (pd && payload) {
        do_processing(thread_id, std::ref(*pd.get()), std::ref(*payload.get()));
      } else {
        npulog(cout << "Something went wrong packet and/or pd were null"
                    << endl;)
      }
      halport->SendtoODE(thread_id, pd, payload);
    }
}

void ApplicationLayer::do_processing(std::size_t thread_id,
                                     PacketDescriptor& pd, Packet& payload) {
  uint32_t counter = 0;
  std::string ApplicationName = SimulationParameters["application_name"].get();
  std::string npu_name;
  for (std::string temp : parent_->ModuleHierarchy()) {
    if (temp.find("npu") != std::string::npos) {
      npu_name = temp;
    }
  }
  auto received_p = call_application(ApplicationName)(npu_name,
      counter, std::ref(pd), std::ref(payload), nullptr);
  payload = received_p;
  npulog(profile, cout << "Loop in AL:do_processing" << endl;)
  wait(counter, SC_NS);
}
