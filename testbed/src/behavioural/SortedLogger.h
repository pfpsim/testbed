#ifndef BEHAVIOURAL_REORDERLOGGER_H_
#define BEHAVIOURAL_REORDERLOGGER_H_
#include <string>
#include <vector>
#include "../structural/SortedLoggerSIM.h"
#include "common/Packet.h"

class SortedLogger: public SortedLoggerSIM {
 public:
  SC_HAS_PROCESS(SortedLogger);
  /*Constructor*/
  explicit SortedLogger(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~SortedLogger();

 public:
  void init();

 private:
  void SortedLogger_PortServiceThread();
  void SortedLoggerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  std::vector<std::shared_ptr<Packet>> packets;
  std::string logfilename;
};

#endif  // BEHAVIOURAL_REORDERLOGGER_H_
