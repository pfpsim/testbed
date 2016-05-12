#ifndef BEHAVIOURAL_SCHEDULER_H_
#define BEHAVIOURAL_SCHEDULER_H_
#include <string>
#include <vector>
#include <map>
#include "../structural/SchedulerSIM.h"
#include "CommonIncludes.h"

class Scheduler: public SchedulerSIM {
 public:
  SC_HAS_PROCESS(Scheduler);
  /*Constructor*/
  explicit Scheduler(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT
  /*Destructor*/
  virtual ~Scheduler() = default;

 public:
  void init();

 private:
  void SchedulerThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;

  sc_event cluster_restored_credit_;
  //! Store assignments to TECs
  std::map<std::size_t, std::size_t> cluster_assignment_;
  //! Store credit for each TEC
  std::vector<std::size_t> cluster_credit_;
  std::size_t WS_PE;
};

#endif  // BEHAVIOURAL_SCHEDULER_H_
