#include "./Splitter.h"
#include <string>
#include <chrono>  // NOLINT(build/c++11)
#include <fstream>
#include "common/RoutingPacket.h"
#include "common/routingdefs.h"

Splitter::Splitter(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):SplitterSIM(nm,parent,configfile),outlog(OUTPUTDIR+"IngressTrace.csv") {  // NOLINT
  /*sc_spawn threads*/
  ThreadHandles.push_back(sc_spawn(sc_bind(&Splitter::SplitterThread, this, 0))); // NOLINT
}

void Splitter::init() {
  init_SIM(); /* Calls the init of sub PE's and CE's */
  add_counter("PKT_CNT");
  int isolation_groups = GetParameter("isolation_groups").get();
  for (std::size_t ig = 0; ig < isolation_groups; ig++) {
    add_counter("IG" + std::to_string(ig) + "_PKT_CNT");
  }
}

void Splitter::SplitterThread(std::size_t thread_id) {
  int counter1 = 0;
  int counter2 = 0;

  bool cpagentflag = false;

  auto seed = std::chrono::high_resolution_clock::now()
        .time_since_epoch().count();
  std::mt19937 rng(seed);
  int isolation_groups = SimulationParameters["isolation_groups"].get();
  std::uniform_int_distribution<std::size_t>
            uid_isolation_groups(0, isolation_groups - 1);

  int packet_id = 0;

  while (1) {
    if (cpagentflag == true) {
      npulog(profile, cout <<"Starting to accept from ingress" <<endl;)
      npulog(minimal,
      cout << On_ICyan << "Starting to accept from ingress" << txtrst << endl;)
      while (1) {
        // 1. Receive Testbed packet from ingress
        auto testbed_packet = std::dynamic_pointer_cast
                                <TestbedPacket>(ingress->get());
        npulog(profile, cout << "Checking received Testbed packet at splitter"
        << endl << "Packet length: " << testbed_packet->getData().size()
        << endl << "Expected: " << sizeof(ether_header) + sizeof(ip) +
          sizeof(udphdr) + 1  // the 1 is the request byte
        << endl << "Source IP: "
        << inet_ntoa(((struct ip*)(testbed_packet->getData().data() +
        sizeof(ether_header)))->ip_src)
        << endl << "Destination IP: "
        << inet_ntoa(((struct ip*)(testbed_packet->getData().data() +
        sizeof(ether_header)))->ip_dst) << endl;)
        packet_id++;

        // Convert testbed packet to npu packet
        std::size_t isolation_group = uid_isolation_groups(rng);
        size_t context = 1;
        size_t priority = -1;

        auto npu_packet = std::make_shared<Packet>(packet_id,
              context, priority, testbed_packet->getData());

        npulog(profile, cout << "Checking npu packet: " << npu_packet->id()
        << " - " << npu_packet->context_id() << " - "
        << npu_packet->packet_priority() << " - " << npu_packet->data().size()
        << endl;)

        npulog(debug,
        cout << dec <<"*&- Splitter pktin Counter1: "<< counter1
             << " FMG allocation Counter2: "<< counter2<< endl;)

        /* Profiling */
        npulog(profile,
          cout << "received Packet" << IBlue << npu_packet->id() << txtrst
               << " from ingress" << endl;)
        npulog(minimal,
          cout << "received Packet" << IBlue << npu_packet->id() << txtrst
               << " from ingress" << endl;)
        outlog << npu_packet->id() << ","
               << sc_time_stamp().to_default_time_units() << endl;
        counter1++;

        // 1.1. Extract data
        increment_counter
        ("IG" + std::to_string(isolation_group) + "_PKT_CNT");

        /* TODO:p4
        // calculate the actual length of the header part of the payload
        phv_data_t * phv;
        size_t packet_length = received_packet.data().size();
        p4_do_parsing(received_packet.data().data(), packet_length, &phv);
        size_t header_length = packet_length - phv->payload_len;
        // TODO delete that dummy PHV but there's memory leaks everywhere anyways...
         */

        // 2. Split packet payload and header
        auto start_of_header = npu_packet->data().begin();
        // start_of_header + header_length;
        auto end_of_header   = npu_packet->data().end();
        // 2.1 Extract the header part of the packet into a new buffer
        PacketDescriptor::raw_header_t raw_header;
        std::copy
        (start_of_header, end_of_header, std::back_inserter(raw_header));
        // 2.2 remove header from packet, so we store only the payload
        npu_packet->data().erase(start_of_header, end_of_header);
        // 2.3 Create PacketDescriptor
        auto pd = std::make_shared<PacketDescriptor>(
            parent_->module_name(),
            npu_packet->id(),
            npu_packet->context_id(),
            isolation_group,
            raw_header);

        // 2.4 set TTL for each packet
        // pd->TTL(4);

        // 2.4.1 Get vaddr from FMG
        auto memmessage = std::make_shared<IPC_MEM>();
        memmessage->id(npu_packet->id());
        memmessage->RequestType = "ALLOCATE";
        memmessage->bytes_to_allocate = static_cast<int>
                                        (npu_packet->data().size());
        npulog(profile, cout << "Memory manager to allocate "
        << memmessage->bytes_to_allocate << " bytes for packet ID "
        << memmessage->id() << endl;)
        // 2.4.2 Send req to FMG
        auto to_send = make_routing_packet
                       (module_name(), PathtoMemoryManager , memmessage);
        ocn_wr_if->put(to_send);
        npulog(profile, cout << "Splitter sending packet to memory manager"
        << endl;)

        auto fmgreply = unbox_routing_packet<IPC_MEM>(ocn_rd_if->get());
        npulog(profile, cout << "received data from memory manager" << endl;)
        auto ipcpkt = fmgreply->payload;
        // PacketDescriptor
        pd->payload_paddr = ipcpkt->tlm_address;
        pd->payload_target_mem_ = ipcpkt->target_mem_mod;  // routing path
        pd->payload_target_memname_ = ipcpkt->target_mem_name;  // memname
        // payload
        // routing path
        npu_packet->target_mem_dest = ipcpkt->target_mem_mod;
        // memname
        npu_packet->dest_target_memname_ = ipcpkt->target_mem_name;
        npu_packet->payload_paddr = ipcpkt->tlm_address;
        counter2++;
        // 3. Write Payload to IDA
        auto sendtomemory = make_routing_packet
                           (module_name(),                    // Splitter
                           npu_packet->target_mem_dest,  // Target Memory
                           npu_packet);              // The Packet itself
        ocn_wr_if->put(sendtomemory);
        npulog(profile, cout << "Splitter sending to memory"
        << npu_packet->target_mem_dest << endl;)
        wait(1, SC_NS);
        // 4. Write PacketDescriptor to Parser
        auto sendtoparser = make_routing_packet(module_name(), "parser", pd);
        npulog(profile, cout << "Splitter sending to parser" << endl;)
        ocn_wr_if->put(sendtoparser);
      }
    } else {
      auto receivedpkt = ocn_rd_if->get();
      cpagentflag = true;
    }
  }
}
