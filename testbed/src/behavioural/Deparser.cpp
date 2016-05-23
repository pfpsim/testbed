#include "./Deparser.h"
#include <string>
#include "common/RoutingPacket.h"
#include "common/Packet.h"
#include "common/PacketDescriptor.h"

Deparser::Deparser(sc_module_name nm, pfp::core::PFPObject* parent, std::string configfile):DeparserSIM(nm, parent, configfile), outlog(OUTPUTDIR + "EgressTrace.csv") {  // NOLINT(whitespace/line_length)
  /*sc_spawn threads*/
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Deparser::DeparserThread_QueueService, this, 0)));
  ThreadHandles.push_back(
      sc_spawn(sc_bind(&Deparser::Deparser_PortServiceThread, this)));
}

void Deparser::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}

void Deparser::Deparser_PortServiceThread() {
  while (1) {
    auto received_tr = ocn_rd_if->get();
    wait(SC_ZERO, SC_NS);
    if (auto received_rp = try_unbox_routing_packet
                          <PacketDescriptor>(received_tr)) {
      tmguard.lock();
      fromTrafficManagerBuffer.push(received_rp->payload);
      tmguard.unlock();
      bufferevent.notify(sc_core::SC_ZERO_TIME);
    } else if (auto received_rp = try_unbox_routing_packet
                                  <Packet>(received_tr)) {
      memguard.lock();
      fromMEM.push(received_rp->payload);
      memguard.unlock();
      bufferevent.notify(sc_core::SC_ZERO_TIME);
    } else {
      npu_error("Deparser received unknown");
    }
  }
}

void Deparser::DeparserThread_QueueService(std::size_t thread_id) {
  while (1) {
    if (!fromTrafficManagerBuffer.empty()) {
      auto received_pd = fromTrafficManagerBuffer.front();
      fromTrafficManagerBuffer.pop();
      memory_.emplace(received_pd->id(), received_pd);
      // 1. Get data from MCT
      ocn_wr_if->put(make_routing_packet(
                     module_name_,
                     received_pd->payload_target_mem_,
                     received_pd));
    } else if (!fromMEM.empty()) {
      auto p = fromMEM.front();
      fromMEM.pop();
      // 3. deparse the packet header, and re-add it to the outgoing packet
      auto received_pd = memory_.at(p->id());
      auto header = received_pd->deparse(parent_->module_name());
      p->data().insert(p->data().begin(), header.begin(), header.end());
      // 4. Write data to output
      p->packet_priority(received_pd->packet_priority());
      p->setEgressPort(received_pd->header()->get_phv()->get_field
      ("standard_metadata.egress_port").get_int());

      std::shared_ptr<TestbedPacket> testbedPacket =
      std::make_shared<TestbedPacket>();
      testbedPacket->setData().insert(testbedPacket->setData().begin(),
      p->data().begin(), p->data().end());
      testbedPacket->setEgressPort(p->getEgressPort());
      egress->put(testbedPacket);
      npulog(minimal,
        cout << "sent Packet" << Yellow << p->id() << txtrst
             << " to egress" << endl;)
       npulog(profile,
         cout << "sent Packet" << Yellow << p->id() << txtrst
              << " to egress" << endl;)
      // outlog << p->id() << "," << sc_time_stamp().to_default_time_units()
      //       << endl;
    } else {
      wait(bufferevent);
    }
  }
}
