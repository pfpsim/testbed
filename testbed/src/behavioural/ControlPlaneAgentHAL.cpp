#include "./ControlPlaneAgentHAL.h"
#include <string>
#include <vector>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"

ControlPlaneAgentHAL::ControlPlaneAgentHAL(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ControlPlaneAgentHALSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  // TODO(Lemniscate) don't hardcode
  LoadMemoryConfig(CONFIGROOT+"memory_48_12edrams.cfg");
  LoadMemoryMAPConfig(CONFIGROOT+"memorymap_48_12edrams.cfg");
}

void ControlPlaneAgentHAL::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
/*
======== Functions that TLM VAR calls ==============
 */
ControlPlaneAgentHAL::TlmType
ControlPlaneAgentHAL::tlmallocate(int BytestoAllocate) {
  auto memmessage = std::make_shared<IPC_MEM>();
  memmessage->id(3146);
  memmessage->RequestType = "ALLOCATE";
  memmessage->bytes_to_allocate = BytestoAllocate;
  // TODO(gordon/Lemniscate) SET function is in ControlPlaneAgent
  memmessage->table_name = "TODO";

  ocn_wr_if->put(make_routing_packet(
                 GetParent()->module_name(), PathtoMemoryManager, memmessage));
  // wait for reply from fmg
  auto received_tr = unbox_routing_packet<IPC_MEM>(ocn_rd_if->get());
  auto ipcpkt = received_tr->payload;
  return ipcpkt->tlm_address;
}

void
ControlPlaneAgentHAL::tlmwrite(int VirtualAddress, int data, TlmType size) {
  // 1. Find Where does this write go ?
  memdecode result = decodevirtual(VirtualAddress);
  // 2. Write to mem + shadow edmems if decode return addr lies in edram region
  if (result.memname.find("ed") != std::string::npos) {
    std::vector<std::string> paths_to_mems = getmempaths_map();
    for (std::vector<std::string>::iterator it = paths_to_mems.begin();
         it!= paths_to_mems.end(); ++it) {
  // 2.1 If ed set write to each
    std::string destination  = *it;
    auto memmessage = std::make_shared<IPC_MEM>();
    memmessage->id(3146);
    memmessage->RequestType = "WRITE";
    memmessage->bytes_to_allocate = data;
    memmessage->tlm_address = result.physcialaddr;
    ocn_wr_if->write(make_routing_packet
                      (GetParent()->module_name(), destination, memmessage));
    }
  } else if (result.memname.find("mct") != std::string::npos) {
    auto memmessage = std::make_shared<IPC_MEM>();
    memmessage->id(3147);
    memmessage->RequestType = "WRITE";
    memmessage->bytes_to_allocate = data;
    memmessage->tlm_address = result.physcialaddr;
    ocn_wr_if->write(make_routing_packet
                    (GetParent()->module_name(), result.mempath, memmessage));
  } else {
    npu_error("Unknown Decode -ControlPlane AGENT: -"+result.memname);
  }
  // 3. Return Control
  return;
}

ControlPlaneAgentHAL::TlmType
ControlPlaneAgentHAL::tlmread(int VirtualAddress) {
  memdecode result = decodevirtual(VirtualAddress);
  auto memmessage = std::make_shared<IPC_MEM>();
  memmessage->id(3148);
  memmessage->RequestType = "READ";
  memmessage->tlm_address = result.physcialaddr;
  ocn_wr_if->put(make_routing_packet
                (GetParent()->module_name(), result.mempath, memmessage));

  auto received_tr = unbox_routing_packet<IPC_MEM>(ocn_rd_if->get());
  auto ipcpkt = received_tr->payload;
  return ipcpkt->bytes_to_allocate;
}
