/*
 * testbed: Simulation environment for PFPSim Framework models
 *
 * Copyright (C) 2016 Concordia Univ., Montreal
 *     Samar Abdi
 *     Umair Aftab
 *     Gordon Bailey
 *     Faras Dewal
 *     Shafigh Parsazad
 *     Eric Tremblay
 *
 * Copyright (C) 2016 Ericsson
 *     Bochra Boughzala
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef BEHAVIOURAL_COMMON_TESTBEDUTILITIES_H_
#define BEHAVIOURAL_COMMON_TESTBEDUTILITIES_H_
#include <arpa/inet.h>
#include <string>
#include <bitset>
#include <map>
#include <vector>
#include <random>
#include <iostream>
#include <chrono>  // NOLINT(build/c++11)
#include "systemc.h"  // NOLINT(build/include)
#include "./TestbedPacket.h"

struct DistributionType {
  std::string type;
  double param1;
  double param2;
};
struct DelaysType {
  std::vector<sc_time> delay_values;
  DistributionType distribution;
};
struct SizesType {
  std::vector<int32_t> size_values;
  DistributionType distribution;
};
struct AddrType {
  std::vector<std::string> prefix_values;
  DistributionType distribution;
};
struct ClientConfigStruct {
  std::string node_type;
  sc_time end_time;
  bool archive;
  std::vector<std::string> list;
  std::vector<std::vector<uint8_t> > header_data;
  DelaysType delay;
};
struct ServerConfigStruct {
  std::string node_type;
  bool archive;
  int mtu;
  int32_t datarate;
  std::vector<std::string> list;
  SizesType fsize;
  size_t sessions;
  AddrType prefixes;
};

/*
 * Structure to store the connection details in clients and servers.
 */
enum ConnectionStates { serverQuery, connectionSetup, fileRequest, fileResponse,
  fileProcessing, idle, connectionTeardown };
struct ConnectionDetails{
  std::vector<uint8_t> received_header;
  size_t file_pending;
  ConnectionStates connection_state;
  sc_time idle_pending;
  sc_time wakeup;
  uint16_t fileIndex;
  uint16_t delayIndex;
  bool active;
};

/*
 * ether_header defined in netinet/ether.h
 * ip defined in netinet/ip.h
 * udphdr defined in netinet/udp.h
 * tcphdr defined in netinet/tcp.h
 * ip6_hdr defined in netinet/ip6.h
 */
struct ether_header {
  u_int8_t ether_dhost[6]; /* destination eth addr  */
  u_int8_t ether_shost[6]; /* source ether addr  */
  u_int16_t ether_type; /* packet type ID field  */
}__attribute__((__packed__));

struct ip {
  u_int8_t ip_vhl; /* version - header length */
  u_int8_t ip_tos; /* type of service */
  u_short ip_len; /* total length */
  u_short ip_id; /* identification */
  u_short ip_off; /* fragment offset field */
#define  IP_RF 0x8000      /* reserved fragment flag */
#define  IP_DF 0x4000      /* dont fragment flag */
#define  IP_MF 0x2000      /* more fragments flag */
#define  IP_OFFMASK 0x1fff    /* mask for fragmenting bits */
  u_int8_t ip_ttl; /* time to live */
  u_int8_t ip_p; /* protocol */
  u_short ip_sum; /* checksum */
  struct in_addr ip_src, ip_dst; /* source and dest address */
};
struct ip6_hdr {
  uint32_t ip6_un1_flow; /* 4 bits version, 8 bits TC, 20 bits flow-ID */
  uint16_t ip6_un1_plen; /* payload length */
  uint8_t ip6_un1_nxt; /* next header */
  uint8_t ip6_un1_hlim; /* hop limit */
  // uint8_t ip6_un2_vfc; /* 4 bits version, top 4 bits tclass */
  struct in6_addr ip6_src; /* source address */
  struct in6_addr ip6_dst; /* destination address */
};
struct trackhdr {
  /*
   * The fields are same as specified
   * in the configuration xmls.
   */
  sc_time created;  // time of packet creation
  struct in_addr ip_src, ip_dst;
  struct in6_addr ip6_src, ip6_dst;
  u_int8_t ether_dhost[6], ether_shost[6];
  u_int8_t ttl;
  u_int16_t sport, dport;
  uint8_t threadID;
};
struct tcphdr {
  u_int16_t th_sport; /* source port */
  u_int16_t th_dport; /* destination port */
  u_int32_t th_seq; /* sequence number */
  u_int32_t th_ack; /* acknowledgement number */
  u_int8_t th_offx; /* data offset - unused */
  u_int8_t th_flags;
# define TH_FIN  0x01
# define TH_SYN  0x02
# define TH_RST  0x04
# define TH_PUSH  0x08
# define TH_ACK  0x10
# define TH_URG  0x20
  u_int16_t th_win; /* window */
  u_int16_t th_sum; /* checksum */
  u_int16_t th_urp; /* urgent pointer */
};
struct udphdr {
  u_int16_t uh_sport; /* source port */
  u_int16_t uh_dport; /* destination port */
  u_int16_t uh_ulen; /* udp length */
  u_int16_t uh_sum; /* udp checksum */
};
struct rtphdr {
  uint8_t rtp_vpxcc;
  uint8_t rtp_mpt;
  uint16_t rtp_seq;
  uint32_t rtp_time;
  uint32_t rtp_ssrc;
};
struct dnshdr {
  uint16_t id;

  unsigned char qr :1;  // 0 query, 1 response
  unsigned char opcode :4;  // 0 standard query
  unsigned char aa :1;  // authoritive answer
  unsigned char tc :1;  // truncated message
  unsigned char rd :1;  // recursion desired

  unsigned char ra :1;  // recursion available
  unsigned char z :3;  // reserved
  unsigned char rcode :4;  // response code

  unsigned q_count :16;  // number of question entries
  unsigned ans_count :16;  // number of answer entries
  unsigned auth_count :16;  // number of authority entries
  unsigned add_count :16;  // number of resource entries
};

class TestbedUtilities {
 public:
  TestbedUtilities() = default;
  ~TestbedUtilities() = default;
  ClientConfigStruct getClientConfigurations(
    std::map<std::string, std::string> configMap, std::string configFile);
  ServerConfigStruct getServerConfigurations(
    std::map<std::string, std::string> configMap, std::string configFile);
  std::vector<std::string> getStringVector(
    const std::string &str);
  void getDefaultDistributionParameters(
    const std::string &distribution,  double *param1, double *param2);

  int getRandomNum(int min, int max, std::string distribution,
    double param1 = 0,
    double param2 = 0);
  int addHeaders(std::shared_ptr<TestbedPacket> pkt,
    const std::vector<uint8_t> &headers);
  void finalizePacket(std::shared_ptr<TestbedPacket> pkt,
    const std::vector<std::string> &headers);
  void addPayload(std::shared_ptr<TestbedPacket> pkt, int len);

  std::string getServerInstanceAddress(const AddrType &addrType,
    const std::map<std::string, size_t> &sessions, int initIndex);

  // All our DNS packets will be UDP packets for the sake of simplicity
  // 1. type = 0 : DNS Query
  // 2. type = 1 : DNS Response
  void getDnsPacket(std::shared_ptr<TestbedPacket> reqPacket,
    std::shared_ptr<TestbedPacket> resPacket, int type,
    const std::vector<std::string> &headers, const std::string &message);
  std::string  getDNSResponse(std::shared_ptr<TestbedPacket> pkt,
      const std::vector<std::string> &headers);
  // 1. Servers send a SYN/ACK packet                                   type -1
  // 2. Servers/Clients send a ACK packet                                type -2
  // 3. Servers/Clients send a PSH packet                                type -3
  // 4. Servers send a FIN/ACK packet                                    type -4
  // 5. Clients send a RST packet                                        type -5
  // 6. Servers/Servers send a ACK pakcet for a TCP payload of size N    type +N
  void getResponseHeader(std::shared_ptr<TestbedPacket> reqPacket,
    std::shared_ptr<TestbedPacket> resPacket, int type,
    const std::vector<std::string> &headers);

  uint16_t calculateIPChecksum(void* vdata, int length);
  uint16_t calculateTCPChecksum(std::shared_ptr<TestbedPacket> pkt, int hdrPos,
    uint8_t protocol, struct ip *ipptr);
  std::vector<std::string> getBaseIPs(const AddrType &addrType);
  // util.updateAddress(received_packet, ncs.list, it->second, "dst");
  void updateAddress(std::shared_ptr<TestbedPacket> pkt,
    const std::vector<std::string> &headers, const std::string &newAddress,
    const std::string &type);

  std::vector<std::string> getIPv4List(std::string dhcpPool, int maxListSize,
    int initIndex);
  std::vector<std::string> interleaveVectors(
    const std::vector<std::vector<std::string> >& cps2);

  std::string getIPAddress(const std::vector<uint8_t> &packet,
    const std::vector<std::string> &list, const std::string &type);
  size_t getHeaderLength(const std::vector<std::string> &headers);
  // https://en.wikipedia.org/wiki/List_of_network_protocols_%28OSI_model%29
  std::vector<uint8_t> getLayer4Header(const std::vector<uint8_t> &packet,
    const std::vector<std::string> &headers);

  void dissectPacket(const std::vector<uint8_t> &packet_data,
    const std::vector<std::string> &headers);

  void getLoadBalancerPacket(std::shared_ptr<TestbedPacket> lb_packet,
    const std::map<std::string, size_t> &server_sessions,
    const std::string &node_id,
    const std::string &controller_ip,
    std::shared_ptr<TestbedPacket> received_packet,
    const std::vector<std::string> &headers);
};

#endif  // BEHAVIOURAL_COMMON_TESTBEDUTILITIES_H_
