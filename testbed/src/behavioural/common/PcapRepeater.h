#ifndef BEHAVIOURAL_COMMON_PCAP_REPEATER_H_
#define BEHAVIOURAL_COMMON_PCAP_REPEATER_H_

#include <pcap/pcap.h>
#include <vector>
#include <cstdint>
#include <string>

class PcapRepeater {
 public:
  explicit PcapRepeater(std::string inputfile);
  ~PcapRepeater();

  bool hasNext();
  std::vector<uint8_t> getNext();

 private:
  pcap_t        * input;
  struct pcap_pkthdr pkt_header;
  const uint8_t * pkt_data;
};

#endif  // BEHAVIOURAL_COMMON_PCAP_REPEATER_H_
