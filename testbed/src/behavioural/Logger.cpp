#include "./Logger.h"
#include <string>

Logger::Logger(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):LoggerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Logger::Logger_PortServiceThread, this)));
}

void Logger::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void Logger::Logger_PortServiceThread() {
  std::string outputfile;
  if (SimulationParameters["use-validation-out"].get().get<bool>()) {
    outputfile = SPARG("validation-out");
  } else {
    outputfile = "output.pcap";
  }

  PcapLogger logger(outputfile);

  while (1) {
    auto p = in->get();
    auto p1 = std::dynamic_pointer_cast<Packet>(p);
    if (p1) {
      logger.logPacket(p1->data());
    } else {
      npu_error("Logger Packet Dynamic Cast Failed");
    }
    out->put(p);
  }

}
