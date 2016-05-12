#ifndef BEHAVIOURAL_COMMON_PCAPLOGGER_H_
#define BEHAVIOURAL_COMMON_PCAPLOGGER_H_

#include <stdint.h>
#include <pcap/pcap.h>
#include <vector>
#include <string>
#include "systemc.h"

class PcapLogger {
 public:
  explicit PcapLogger(std::string outputfile);
  ~PcapLogger();

  void logPacket(std::vector<uint8_t> & data);
  void logPacket(const std::vector<uint8_t> &data, sc_time currTime);

 private:
  pcap_dumper_t * dumper;
};

#endif  // BEHAVIOURAL_COMMON_PCAPLOGGER_H_
