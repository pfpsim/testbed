#include "./SortedLogger.h"
#include <string>
#include <vector>
#include <memory>
#include <string>
#include "common/Packet.h"
#include "common/PcapLogger.h"

SortedLogger::SortedLogger(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):SortedLoggerSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
  sc_spawn(sc_bind(&SortedLogger::SortedLogger_PortServiceThread,this));
}

void SortedLogger::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  logfilename = SPARG("validation-out");
}

namespace {
class PacketLessThan {
 public:
  bool operator() (const std::shared_ptr<Packet>& a,
                   const std::shared_ptr<Packet>& b) {
    return a->id() < b->id();
  }
};
}  // anonymous namespace

void SortedLogger::SortedLogger_PortServiceThread(){

  PacketLessThan packet_gt;

  size_t next_id = 0;
  while(1){
    auto p = in->get();
    if(auto p1 = std::dynamic_pointer_cast<Packet>(p)){
      auto it = std::lower_bound(packets.begin(), packets.end(), p1, packet_gt);
      packets.insert(it, p1);
    }
    else{
      std::cerr<<"Logger Packet Dynamic Cast Failed"<<endl;
      SC_REPORT_ERROR("PACKET LOGGER","Logger Dynamic Cast Failed");
      sc_stop();
    }
  }
}

SortedLogger::~SortedLogger() {
  PcapLogger logger(logfilename);

  for (auto & p : packets) {
    logger.logPacket(p->data());
  }

}
