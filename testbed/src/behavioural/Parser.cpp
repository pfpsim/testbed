#include "./Parser.h"
#include <string>
#include <vector>
#include <chrono>

#include "common/RoutingPacket.h"

Parser::Parser(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):ParserSIM(nm, parent, configfile) {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  int isolation_groups = GetParameter("isolation_groups").get();
  for (std::size_t rows = 0; rows < isolation_groups; rows++) {
    std::vector<std::size_t> new_row;
    credit_matrix_.push_back(new_row);
  }
  int rcos = GetParameter("rcos").get();
  int credit = GetParameter("aam_credit").get();
  for (std::size_t row = 0; row < credit_matrix_.size(); row++) {
    for (std::size_t col = 0; col < rcos; col++) {
      credit_matrix_[row].push_back(credit);
    }
  }
  ThreadHandles.push_back(sc_spawn(sc_bind(&Parser::ParserThread, this, 0)));
}

void Parser::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  add_counter("PCL_PKT_FROM_PDM_ANY_EVENT");
  int isolation_groups = GetParameter("isolation_groups").get();
  for (std::size_t ig = 0; ig < isolation_groups; ig++) {
    add_counter("PCL_PKT_FROM_PDM_IG" + std::to_string(ig) + "_EVENT");
    add_counter("PCL_PKT_TO_ODE_IG" + std::to_string(ig) + "_EVENT");
    add_counter("PCL_PKT_TO_GJQ_IG" + std::to_string(ig) + "_EVENT");
  }
  add_counter("PCL_AAM_RSC_CHK_REQ_EVENT");
  add_counter("PCL_AAM_RSC_CHK_RPL_EVENT");
}

void Parser::ParserThread(std::size_t thread_id) {
  while (1) {
    auto received_rp = unbox_routing_packet
                       <PacketDescriptor>(ocn_rd_if->get());
    auto received_pd = received_rp->payload;
    if (received_rp->source == "splitter") {
      increment_counter("PCL_PKT_FROM_PDM_ANY_EVENT");
      increment_counter("PCL_PKT_FROM_PDM_IG" +
                        std::to_string(received_pd->isolation_group()) +
                        "_EVENT");
      // 2. Check for "pcl_aam" port
      if (!ocn_wr_if->nb_can_put()) {
#ifdef InModuleDrop
        drop_data(received_pd, "pcl_vcc");
#else
        wait(ocn_wr_if->ok_to_put());
#endif
      } else {
        // Writing part
        auto seed = static_cast<unsigned int>
        (std::chrono::high_resolution_clock::now()
          .time_since_epoch().count());
        std::mt19937 rng(seed);

        int rcos = SimulationParameters["rcos"].get();
        int priorities = SimulationParameters["priorities"].get();
        std::uniform_int_distribution<std::size_t> uid_rcos(0, rcos - 1);
        std::uniform_int_distribution<std::size_t> uid_pri(0, priorities - 1);

        // 0. Parse the packet
        received_pd->parse(parent_->module_name());
        // 1. Choose priority for received PacketDescriptor
        // When a packet is enqueued,
        // it calculates the priority number, i.e. a number between 1 and 5.
        auto pri = received_pd->context() % priorities;
        // 2. Assing Resource Class of Service to PacketDescriptor
        received_pd->resource_class_of_service(uid_rcos(rng));
        // 3. Assign prioirty to PacketDescriptor
        // Each packet enqueued to the queuing
        // discipline is assigned a priority.
        received_pd->packet_priority(pri);
        increment_counter("PCL_AAM_RSC_CHK_REQ_EVENT");
        std::size_t row = received_pd->isolation_group();
        std::size_t col = received_pd->resource_class_of_service();
        if (credit_matrix_.at(row).at(col) > 0) {
          // 2a. Allow the Packet to pass and decrement credit
          auto credit = credit_matrix_.at(row).at(col);
          credit--;
          credit_matrix_.at(row).at(col) = credit;
          increment_counter("AAM_ADMIT_F_EVENT");
        } else {
          // 2b. Flag the PacketDescriptor so that the
          // Packet is dropped due to congestion
          received_pd->drop(true);
        }
        // 2. Check if Packet needs to be dropped
        if (received_pd->drop()) {
          // 2a. If so, write PacketDescriptor to ODE
          ocn_wr_if->put(make_routing_packet
                        (module_name_, "ode", received_pd));
          increment_counter("PCL_PKT_TO_ODE_IG" + std::to_string
                            (received_pd->isolation_group()) + "_EVENT");
        } else {
          // 2b. If not, pass PacketDescriptor to GJQ
          ocn_wr_if->put(make_routing_packet
                        (module_name_, "scheduler", received_pd));
          increment_counter("PCL_PKT_TO_GJQ_IG" + std::to_string
                                (received_pd->isolation_group()) + "_EVENT");
        }
      }
    } else {
      std::size_t row = received_pd->isolation_group();
      std::size_t col = received_pd->resource_class_of_service();
      auto credit = credit_matrix_.at(row).at(col);
      credit++;
      credit_matrix_.at(row).at(col) = credit;
    }
  }
}
