#ifndef BEHAVIOURAL_SPLITTER_H_
#define BEHAVIOURAL_SPLITTER_H_
#include <string>
#include <vector>
#include "../structural/SplitterSIM.h"
#include "CommonIncludes.h"

class Splitter: public SplitterSIM {
 public:
  SC_HAS_PROCESS(Splitter);
  /*Constructor*/
  explicit Splitter(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~Splitter() = default;

 public:
  void init();

 private:
  void SplitterThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;
  std::ofstream outlog;
};

#endif  // BEHAVIOURAL_SPLITTER_H_
