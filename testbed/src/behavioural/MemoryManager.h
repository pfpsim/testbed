#ifndef BEHAVIOURAL_MEMORYMANAGER_H_
#define BEHAVIOURAL_MEMORYMANAGER_H_
#include <string>
#include <vector>
#include "../structural/MemoryManagerSIM.h"
#include "pfpsim/pfpsim.h"
#include "CommonIncludes.h"

class MemoryManager: public MemoryManagerSIM, public MemoryUtilities {
 public:
  SC_HAS_PROCESS(MemoryManager);
  /*Constructor*/
  explicit MemoryManager(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~MemoryManager() = default;

 public:
  void init();

 private:
  void MemoryManager_PortServiceThread();
  void MemoryManagerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  tlm_addr_ TlmAllocate(tlm_addr_ size_of_data);

  std::vector<tlm_addr_> counter;
  tlm_addr_ addrcounter = 0;
  enum{ SIZE1 = 2};
  enum{ SIZE2 = 16192};
  int totalsize = SIZE1 + SIZE2;

  std::string setsourcetome_;
  std::string parentmod_;
  std::ofstream outlog;

  unsigned long wordsizetoBytes(std::size_t wordsize);  // NOLINT(runtime/int)
};

#endif  // BEHAVIOURAL_MEMORYMANAGER_H_
