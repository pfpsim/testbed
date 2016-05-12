#include "PcapRepeater.h"
#include <algorithm>
#include <vector>
#include <string>
#include "pfpsim/pfpsim.h"

PcapRepeater::PcapRepeater(std::string inputfile) {
  char errbuf[PCAP_ERRBUF_SIZE];

  input = pcap_open_offline((inputfile).c_str(), errbuf);

  if (!input) {
    std::cout << "Could not open pcap_input file "
              << inputfile
              << "(" << errbuf << ")" << std::endl;
    assert(input);
  }

  pkt_data = pcap_next(input, &pkt_header);
  }

PcapRepeater::~PcapRepeater() {
  pcap_close(input);
}

bool PcapRepeater::hasNext() {
  return !!pkt_data;
}

std::vector<uint8_t> PcapRepeater::getNext() {
  std::vector<uint8_t> vec(pkt_header.caplen);
  if (pkt_data) {
    std::copy(pkt_data, pkt_data + pkt_header.caplen, vec.begin());
    pkt_data = pcap_next(input, &pkt_header);
  }

  return vec;
}
