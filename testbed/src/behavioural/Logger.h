#ifndef BEHAVIOURAL_LOGGER_H_
#define BEHAVIOURAL_LOGGER_H_
#include <string>
#include <vector>
#include "../structural/LoggerSIM.h"
#include "CommonIncludes.h"

class Logger: public LoggerSIM {
 public:
  SC_HAS_PROCESS(Logger);
  /*Constructor*/
  explicit Logger(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Logger() = default;

 public:
  void init();

 private:
  void Logger_PortServiceThread();
  std::vector<sc_process_handle> ThreadHandles;
};

#endif  // BEHAVIOURAL_LOGGER_H_
