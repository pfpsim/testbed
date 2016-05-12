#include "./MemoryManager.h"
#include <string>
#include "common/RoutingPacket.h"

MemoryManager::MemoryManager(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):MemoryManagerSIM(nm, parent, configfile), outlog(OUTPUTDIR+"MemoryManagerAllocationTrace.csv") {  // NOLINT(whitespace/line_length)
    /*sc_spawn threads*/
  ThreadHandles.push_back
      (sc_spawn(sc_bind(&MemoryManager::MemoryManagerThread, this, 0)));
}

void MemoryManager::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  setsourcetome_ = "cluster_0."+module_name_;
  parentmod_ = "cluster_0";
  LoadMemoryConfig(CONFIGROOT+"memory_48_12edrams.cfg");
  for (auto memorynames : memnames) {
    add_counter(memorynames+"Usage");
  }
}
void MemoryManager::MemoryManager_PortServiceThread() {
  // Thread function to service input ports.
}
void MemoryManager::MemoryManagerThread(std::size_t thread_id) {
  while (1) {
    auto received = unbox_routing_packet<IPC_MEM>
                                        (cluster_local_switch_rd_if->get());
    auto ipcpkt = received->payload;

      if (ipcpkt->RequestType.find("ALLOCATE") !=std::string::npos) {
        npulog(profile, cout << "Memory manager received a request" << endl;)
        tlm_addr_ virtualaddr = TlmAllocate(ipcpkt->bytes_to_allocate);
        memdecode dest = decodevirtual(virtualaddr);

        npulog(profile, cout <<"!& --- MemoryManager allocation completed:"
          << endl
          << "   --- alloc req from:" <<received->source <<endl
          << "   --- vaddr:" <<virtualaddr <<endl
          << "   --- paddr:" <<dest.physcialaddr <<endl
          << "   --- dest :" <<dest.memname <<endl
          << "   --- free addr head at :" <<addrcounter <<endl
          << "   --- just allocated :" <<ipcpkt->bytes_to_allocate <<endl;)

        outlog << "MemoryManager@" << sc_time_stamp() << ":"
              << "Source," << received->source << ",V," << virtualaddr
              << ",P," << dest.physcialaddr << ",dest," << dest.memname
              << ",free addr head at:," << addrcounter
              << ",TableName:," << ipcpkt->table_name
              << ",Wordallocation," << ipcpkt->bytes_to_allocate
              << ",Byteallocation,"
              << wordsizetoBytes(ipcpkt->bytes_to_allocate)
              <<endl;

        ipcpkt->table_name = "";
        ipcpkt->tlm_address = dest.physcialaddr;
        ipcpkt->target_mem_mod = dest.mempath;
        ipcpkt->target_mem_name = dest.memname;  // memname

        // For Debugger
        increment_counter
        (dest.memname + "Usage", wordsizetoBytes(ipcpkt->bytes_to_allocate));

        auto to_send = make_routing_packet
                       (setsourcetome_, received->source , ipcpkt);

        cluster_local_switch_wr_if->put(to_send);
    } else {
      std::cerr << "UNKNOWN MemoryManager COMMAND" << endl;
    }
  }
}


MemoryManager::tlm_addr_ MemoryManager::TlmAllocate(tlm_addr_ size_of_data) {
  sc_process_handle this_process = sc_get_current_process_handle();
  sc_object* parent = this_process.get_parent_object();
  const char* name = parent->basename();

  if ((addrcounter < VirtualMemMaxSize) &&
      (size_of_data < (VirtualMemMaxSize-addrcounter))) {
    // get size of data to allocate
    double alloc_req = static_cast<float>(size_of_data)/static_cast<float>(32);
    int count = 0;
    int ret_addr = addrcounter;  // current free head of memory block
    for (int i = addrcounter; i < (ceil(alloc_req) + addrcounter); i++) {
      count++;
    }
    addrcounter = addrcounter + count;
    return ret_addr;
  } else {
    std::cerr << "Allocator Ran out of Mem Max VM Addr Range: "
    << (VirtualMemMaxSize-1) <<" Allocated upto addr: " << addrcounter
    <<" Tried to allocate:" <<size_of_data << endl;
    npu_error("Allocator-MemoryManager Ran out of Memory");
  }
}

unsigned long MemoryManager::wordsizetoBytes(std::size_t wordsize) { //NOLINT
  return (unsigned long)ceil((float) wordsize / (float) 32);  // NOLINT
}
