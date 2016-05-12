#include "./TrafficManager.h"
#include <string>
#include <deque>
#include "common/RoutingPacket.h"
#define NO_PRIORITY

TrafficManager::TrafficManager(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):TrafficManagerSIM(nm, parent, configfile), weights({ 5, 3, 2, 2, 2, 1, 1, 1 }) {  // NOLINT(whitespace/line_length)
  for (std::size_t i = 0; i < c; i++) {
    std::deque<std::shared_ptr<PacketDescriptor>> pp;
    packet_order_.push_back(pp);
  }
  int priorities = GetParameter("priorities").get();
  for (std::size_t priority = 0; priority < priorities; priority++) {
    std::deque<std::shared_ptr<PacketDescriptor>> pr;
    packet_order_.push_back(pr);
  }
    /*sc_spawn threads*/
  ThreadHandles.push_back(
    sc_spawn(sc_bind(&TrafficManager::TrafficManagerThread, this, 0)));

  ThreadHandles.push_back(
    sc_spawn(sc_bind(&TrafficManager::TrafficManager_PortServiceThread, this)));
}

void TrafficManager::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void TrafficManager::TrafficManager_PortServiceThread() {
  while (1) {
    auto received_tr = ocn_rd_if->get();
    auto received_pd = std::dynamic_pointer_cast<RoutingPacket<PacketDescriptor>>(received_tr);  // NOLINT
    if (!ocn_wr_if->nb_can_put()) {
#ifdef InModuleDrop
      drop_data(received_pd, "TrafficManagerPortServiceThread");
#else
      wait(ocn_wr_if->ok_to_put());
#endif
    } else {
#ifdef NO_PRIORITY
      output_set_.push(unbox_routing_packet<PacketDescriptor>
                       (received_pd)->payload);
#else
      auto priority = received_pd->packet_priority();
      // 2. Save order in appropriate context queue
      packet_order_.at(priority).push_back(
                unbox_routing_packet<PacketDescriptor>(received_pd)->payload);
#endif
      flag_buffer_empty = false;
      condition.notify();
    }
  }
}

void TrafficManager::TrafficManagerThread(std::size_t thread_id) {
  int priorities = SimulationParameters["priorities"].get();
  while (1) {
    while (flag_buffer_empty) {
      wait(condition);
    }
    /*
    1.Pop the packetDescriptor with highest priority (0: highest)
      and send to pde
    2.When a packet is dequeued it always dequeues from
      the non-empty queuing discipline with the lowest number.
    */
#ifdef FIXED_PRIORITY
    for (auto i = 0; i < (priorities); i++) {
      if (!packet_order_.at(i).empty()) {
        auto to_send = packet_order_.at(i).front();
        packet_order_.at(i).pop_front();
        // Write slected PD to DEPARSER
        auto send = make_routing_packet(module_name(), "deparser", to_send);
        ocn_wr_if->put(send);
        break;
      }
    }
#endif

#ifdef WEIGHTED_ROUND_ROBIN
    for (size_t i = 0; i < priorities; i++) {
      if (c < max_buffer) {
        for (size_t j = 0; j < weights.at(i); j++) {
          if (!packet_order_.at(i).empty() && c < max_buffer) {
            output_set_.push(packet_order_.at(i).front());
            packet_order_.at(i).pop_front();
            c++;
          } else {
            break;
          }
        }
      } else {
        break;
      }
    }
    while (c > 0) {
      auto to_send = output_set_.pop();
      auto send = make_routing_packet(module_name(), "deparser", to_send);
      ocn_wr_if->put(send);
      c--;
    }
#endif
#ifdef NO_PRIORITY
    auto to_send = output_set_.pop();
    auto send = make_routing_packet(module_name(), "deparser", to_send);
    ocn_wr_if->put(send);
    npulog(profile, cout << "wrote " << to_send->id() << "to:deparser" << endl;)
#endif
    if (
      std::all_of(
        packet_order_.begin(),
        packet_order_.end(),
        [](const std::deque<std::shared_ptr<PacketDescriptor>>& packet_queue) {
          return packet_queue.empty();
        })) {
      flag_buffer_empty = true;
    }
  }
}
