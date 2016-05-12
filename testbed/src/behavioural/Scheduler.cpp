#include "./Scheduler.h"
#include <string>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"

Scheduler::Scheduler(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):SchedulerSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Scheduler::SchedulerThread, this, 0)));
}

void Scheduler::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  int clusters = GetParameter("clusters").get();
  int cores = GetParameter("cores").get();
  int core_threads = GetParameter("core_threads").get();
  for (std::size_t cluster = 0; cluster < clusters; cluster++) {
      cluster_credit_.push_back(cores*core_threads);
  }

  WS_PE = 0;
  add_counter("SCHEDULER_ENQ_ANY_EVENT");
  add_counter("SCHEDULER_ENQ_PARSER_IG0_EVENT");
  add_counter("SCHEDULER_ENQ_PARSER_IG1_EVENT");
  add_counter("SCHEDULER_ENQ_PARSER_IG2_EVENT");
  add_counter("SCHEDULER_ENQ_PARSER_IG3_EVENT");
  add_counter("SCHEDULER_DEQ_ANY_EVENT");
  add_counter("SCHEDULER_DEQ_IG0_EVENT");
  add_counter("SCHEDULER_DEQ_IG1_EVENT");
  add_counter("SCHEDULER_DEQ_IG2_EVENT");
  add_counter("SCHEDULER_DEQ_IG3_EVENT");
}

void Scheduler::SchedulerThread(std::size_t thread_id) {
  while (1) {
    // Reading From Input Ports
    auto received_tr = unbox_routing_packet
                       <PacketDescriptor>(ocn_rd_if->get());
    // Writing part
    auto received_pd = received_tr->payload;
    // Check if it is ok to write to output ports
    if (!ocn_wr_if->nb_can_put()) {
#ifdef InModuleDrop
      drop_data(received_pd, "ocn_wr_if");
#else
      wait(ocn_wr_if->ok_to_put());
#endif
    } else {
      if (received_tr->source == "parser") {
        if (!received_pd->drop()) {
          // Assign and write PacketDescriptor to TEC with highest credit
          // TODO(shafigh): Check if this scheduling is correct
          std::size_t cluster_number = 0;
          // to randomize it more for less number of packets
          for (std::size_t mce = 0; mce < cluster_credit_.size(); mce++) {
            if (cluster_credit_.at(mce) >
                cluster_credit_.at(cluster_number)) {
              cluster_number = mce;
            }
          }
          std::size_t no_of_cores = SimulationParameters["cores"].get();
          WS_PE = (WS_PE +1) % no_of_cores;

          // Write to ROC (for ordering)
          ocn_wr_if->put(make_routing_packet
                        (module_name(), "roc", received_pd));
          // Send to core in cluster
          ocn_wr_if->put(make_routing_packet(
                          module_name(),
                          cluster_prefix + std::to_string(cluster_number)
                                  + "."+core_prefix+std::to_string(WS_PE),
                          received_pd));
          increment_counter("SCHEDULER_ENQ_ANY_EVENT");
          increment_counter("SCHEDULER_ENQ_PARSER_IG"
                + std::to_string(received_pd->isolation_group()) + "_EVENT");
          // Save job assignment
          cluster_assignment_.emplace(received_pd->id(), cluster_number);
          // Decrement credit for that TEC
          cluster_credit_.at(cluster_number) -= 1;
        }
      } else {
        // 1. No Drop
        // 2. Check which Cluster was assigned the job
        auto cluster_assignment = cluster_assignment_.at(received_pd->id());
        // 3. Increment credit for that cluster
        cluster_credit_.at(cluster_assignment) += 1;
        cluster_restored_credit_.notify();
        increment_counter("SCHEDULER_DEQ_ANY_EVENT");
        increment_counter("SCHEDULER_DEQ_IG" +
                  std::to_string(received_pd->isolation_group()) + "_EVENT");
      }
    }
  }
}
