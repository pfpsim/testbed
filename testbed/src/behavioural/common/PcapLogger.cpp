#include "PcapLogger.h"
#include <sys/time.h>
#include <string>
#include <vector>
#include "pfpsim/pfpsim.h"

PcapLogger::PcapLogger(std::string outputfile) {
  pcap_t * dummy = pcap_open_dead(DLT_EN10MB, 65535);
  dumper         = pcap_dump_open(dummy, outputfile.c_str());
}

void PcapLogger::logPacket(std::vector<uint8_t> & data) {
  struct pcap_pkthdr pkt_header;
  gettimeofday(&pkt_header.ts, NULL);
  pkt_header.len = pkt_header.caplen = data.size();
  pcap_dump((u_char *)dumper, &pkt_header, data.data());  // NOLINT
}

PcapLogger::~PcapLogger() {
  pcap_dump_close(dumper);
}
void PcapLogger::logPacket(const std::vector<uint8_t> &data, sc_time currTime) {
  struct pcap_pkthdr pkt_header;
  double time = currTime.to_seconds();
  pkt_header.ts.tv_sec = time;
  pkt_header.ts.tv_usec = (time - static_cast<int>(time)) * 1000000;

  pkt_header.len = pkt_header.caplen = data.size();

  pcap_dump((u_char*)dumper, &pkt_header, data.data());  // NOLINT
}
