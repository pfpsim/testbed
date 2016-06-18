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

#include "TestbedUtilities.h"
#include <utility>
#include <string>
#include <vector>
#include <map>

ClientConfigStruct TestbedUtilities::getClientConfigurations(
  pfp::core::PFPObject* node, std::string configFile) {
  ClientConfigStruct ncs;
  try {
    json node_params = node->GetParameter("NODE").get();
    json connection_delays = node->GetParameter("CONNECTION_DELAYS").get();
    json packet_fields = node->GetParameter("PACKET_FIELDS").get();
    // json application_delays = node->GetParameter("APPLICATION_DELAYS").get();
    json ip_address = node_params["ip_address"];

    ncs.node_type = node_params["type"];
    if (ncs.node_type.compare("client") != 0) {
      std::cerr << "Incorrect config file " << configFile
        << " used for client configuration!" << std::endl;
      assert(false);
    }

    auto simulation_time = getStringVector(node_params["simulationTime"]);
    if (simulation_time.size() < 2) {
      std:cerr << configFile << ": Incorrect simTime specified! Usage."
      <<" \"12 SC_SEC\" "
      <<std::endl;
      assert(false);
    }
    if (simulation_time.at(1).compare("SEC") == 0) {
      ncs.end_time = sc_time(stod(simulation_time.at(0)), SC_SEC);
    } else if (simulation_time.at(1).compare("MS") == 0) {
      ncs.end_time = sc_time(stod(simulation_time.at(0)), SC_MS);
    } else if (simulation_time.at(1).compare("US") == 0) {
      ncs.end_time = sc_time(stod(simulation_time.at(0)), SC_US);
    } else if (simulation_time.at(1).compare("NS") == 0) {
      ncs.end_time = sc_time(stod(simulation_time.at(0)), SC_NS);
    } else if (simulation_time.at(1).compare("PS") == 0) {
      ncs.end_time = sc_time(stod(simulation_time.at(0)), SC_PS);
    } else if (simulation_time.at(1).compare("FS") == 0) {
      ncs.end_time = sc_time(stod(simulation_time.at(0)), SC_FS);
    } else {
      std::cerr << configFile << ": Server/ Client Unknown unit for delay "
        << "specified!"
        << " \nSupported values are: SC_SEC, SC_MS, SC_US, SC_NS, SC_PS, SC_FS"
        << std::endl;
      assert(false);
    }
    ncs.archive = node_params["archive"];

    auto headers_list = packet_fields["headers"];
    ncs.list.clear();
    for (std::string hdr : headers_list) {
      ncs.list.push_back(hdr);
    }
    if (ncs.list.size() == 0) {
      std::cerr << "No headers specified!" << std::endl;
      assert(false);
    }

    auto conn_delay_values = connection_delays["values"];
    std::string conn_delay_unit = connection_delays["unit"];
    for (int index = 0; index < conn_delay_values.size(); index++) {
      if (conn_delay_unit.compare("SEC") == 0) {
        ncs.connection_delay.delay_values.push_back(
            sc_time(conn_delay_values.at(index), SC_SEC));
      } else if (conn_delay_unit.compare("MS") == 0) {
        ncs.connection_delay.delay_values.push_back(
            sc_time(conn_delay_values.at(index), SC_MS));
      } else if (conn_delay_unit.compare("US") == 0) {
        ncs.connection_delay.delay_values.push_back(
            sc_time(conn_delay_values.at(index), SC_US));
      } else if (conn_delay_unit.compare("NS") == 0) {
        ncs.connection_delay.delay_values.push_back(
            sc_time(conn_delay_values.at(index), SC_NS));
      } else if (conn_delay_unit.compare("PS") == 0) {
        ncs.connection_delay.delay_values.push_back(
            sc_time(conn_delay_values.at(index), SC_PS));
      } else if (conn_delay_unit.compare("FS") == 0) {
        ncs.connection_delay.delay_values.push_back(
            sc_time(conn_delay_values.at(index), SC_FS));
      } else {
        std::cerr << "Server/ Client Unknown unit for delay specified! \n"
            << "Supported values are: SC_SEC, SC_MS, SC_US,"
            << "SC_NS, SC_PS, SC_FS" << std::endl;
        assert(false);
      }
    }
    auto cd_policy_config = getStringVector(connection_delays["policy"]);
    ncs.connection_delay.distribution.type = cd_policy_config.at(0);
    if (cd_policy_config.size() < 2) {
      getDefaultDistributionParameters(ncs.connection_delay.distribution.type,
          &ncs.connection_delay.distribution.param1,
          &ncs.connection_delay.distribution.param2);
    } else if (cd_policy_config.size() < 3) {
      ncs.connection_delay.distribution.param1 = stod(cd_policy_config.at(1));
      ncs.connection_delay.distribution.param2 = 0;
    } else {
      ncs.connection_delay.distribution.param1 = stod(cd_policy_config.at(1));
      ncs.connection_delay.distribution.param2 = stod(cd_policy_config.at(2));
    }
    /*
    auto app_delay_values = application_delays["values"];
    std::string app_delay_unit = application_delays["unit"];
    for (int index = 0; index < app_delay_values.size(); index++) {
      if (app_delay_unit.compare("SEC") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_SEC));
      } else if (app_delay_unit.compare("MS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_MS));
      } else if (app_delay_unit.compare("US") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_US));
      } else if (app_delay_unit.compare("NS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_NS));
      } else if (app_delay_unit.compare("PS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_PS));
      } else if (app_delay_unit.compare("FS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_FS));
      } else {
        std::cerr << "Server/ Client Unknown unit for delay specified! \n"
            << "Supported values are: SC_SEC, SC_MS, SC_US,"
            << "SC_NS, SC_PS, SC_FS" << std::endl;
        assert(false);
      }
    }
    auto ad_policy_config = getStringVector(application_delays["policy"]);
    ncs.application_delay.distribution.type = ad_policy_config.at(0);
    if (ad_policy_config.size() < 2) {
      getDefaultDistributionParameters(ncs.application_delay.distribution.type,
          &ncs.application_delay.distribution.param1,
          &ncs.application_delay.distribution.param2);
    } else if (ad_policy_config.size() < 3) {
      ncs.application_delay.distribution.param1 = stod(ad_policy_config.at(1));
      ncs.application_delay.distribution.param2 = 0;
    } else {
      ncs.application_delay.distribution.param1 = stod(ad_policy_config.at(1));
      ncs.application_delay.distribution.param2 = stod(ad_policy_config.at(2));
    }
    */

    int instances_count = node_params["instances"];
    std::vector<std::string> clientIPs;  // , serverIPs;
    std::string serverIP;
    auto ip_addr_policy = getStringVector(ip_address["policy"]);

    double cl_dnsp1, cl_dnsp2;
    if (ip_addr_policy.size() < 2) {
      getDefaultDistributionParameters(
        ip_addr_policy.at(0), &cl_dnsp1, &cl_dnsp2);
    } else if (ip_addr_policy.size() < 3) {
      cl_dnsp1 = stod(ip_addr_policy.at(1));
      cl_dnsp2 = 0;
    } else {
      cl_dnsp1 = stod(ip_addr_policy.at(1));
      cl_dnsp2 = stod(ip_addr_policy.at(2));
    }
    std::map<int, int> cl_dnsfreq;
    auto ip_addr_pool = ip_address["pool"];
    std::string ip_addr_policy_type = ip_addr_policy.at(0);
    int rnum = -1;
    for (int index = 0; index < instances_count; index++) {
      if (ip_addr_policy_type.compare("round_robin") == 0) {
        rnum++;
        if (rnum == ip_addr_pool.size()) {
          rnum = 0;
        }
      } else {
        rnum = getRandomNum(0, ip_addr_pool.size() - 1, ip_addr_policy_type,
          cl_dnsp1, cl_dnsp2);
      }
      if (cl_dnsfreq.find(rnum) == cl_dnsfreq.end()) {
        cl_dnsfreq.insert(std::pair<int, int>(rnum, 1));
      } else {
        cl_dnsfreq[rnum]++;
      }
    }
    for (std::map<int, int>::iterator it = cl_dnsfreq.begin();
      it != cl_dnsfreq.end(); ++it) {
      // prefix index: it->first
      // prefix freq : it->second
      std::vector<std::string> ips;
      int initIndex = 1;
      ips = getIPv4List(ip_addr_pool.at(it->first), it->second, initIndex);
      if (ips.size() < it->second) {
        std::cerr << configFile << ": Unable to create sufficient clientIPs to "
        << "satisfy the configured dns policy!" << std::endl;
        assert(false);
      }
      for (std::string tempip : ips) {
        clientIPs.push_back(tempip);
      }
      ips.clear();
    }

    serverIP = node->GetParameter("dns_load_balancer").get();

    for (int index = 0; index < instances_count; index++) {
      std::vector<uint8_t> header;
      for (int nhdr = 0; nhdr < ncs.list.size(); nhdr++) {
        if (ncs.list[nhdr].compare("ethernet_t") == 0) {
          struct ether_header eth_f;
          if (ncs.list.size() >= 2 &&
              ncs.list[nhdr + 1].compare("ipv4_t") == 0) {
            eth_f.ether_type = htons(2048);
          } else if (ncs.list.size() >= 2 &&
              ncs.list[nhdr + 1].compare("ipv6_t") == 0) {
            eth_f.ether_type = 0x86DD;
          } else {
            eth_f.ether_type = htons(0);
          }
          // FOR NOW LEFT DEST MAC AND SOURCE MAC AS GARBAGE
          uint8_t *data =
              static_cast<uint8_t*>(static_cast<void*>(&eth_f));
          header.insert(header.end(), data,
              data + sizeof(struct ether_header));
        } else if (ncs.list[nhdr].compare("ipv4_t") == 0) {
          struct ip ip_f;
          uint8_t vhl = 69;
          ip_f.ip_vhl = vhl;
          int tos = packet_fields["tos"];
          ip_f.ip_tos = (uint8_t) tos;
          uint16_t len = 16;
          ip_f.ip_len = htons(len);
          ip_f.ip_id = htons(
            static_cast<int16_t>(getRandomNum(0, SHRT_MAX, "uniform")));
          ip_f.ip_off = 0;
          int ttl = packet_fields["ttl"];
          ip_f.ip_ttl = (uint8_t) ttl;
          if (ncs.list.size() >=3 && ncs.list[nhdr + 1].compare("tcp_t") == 0) {
            uint8_t proct = 6;
            ip_f.ip_p = proct;
          } else if (ncs.list.size() >=3 &&
              ncs.list[nhdr + 1].compare("udp_t") == 0) {
            uint8_t proct = 17;
            ip_f.ip_p = proct;
          } else {
            ip_f.ip_p = 0;
          }
          ip_f.ip_sum = 0;

          std::string ipsrc;
          ipsrc = clientIPs.at(index);

          if (inet_aton(serverIP.c_str(), &ip_f.ip_dst) == 0) {
            std::cerr << "We got an invalid IP address - 01! Server addr: "
              << serverIP << std::endl;
            assert(false);
          }
          if (inet_aton(ipsrc.c_str(), &ip_f.ip_src) == 0) {
            std::cerr << "We got an invalid IP address - 02! Src: " << ipsrc
                << std::endl;
            assert(false);
          }

          uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&ip_f));
          header.insert(header.end(), data, data + sizeof(struct ip));
        } else if (ncs.list[nhdr].compare("tcp_t") == 0) {
          struct tcphdr tcp_f;
          tcp_f.th_sport = htons((uint16_t)packet_fields["sport"]);
          tcp_f.th_dport = htons((uint16_t)packet_fields["dport"]);
          tcp_f.th_seq = 0;
          tcp_f.th_ack = 0;
          uint8_t offx = 81;
          tcp_f.th_offx = offx;
          // Because we use these configurations for initiation
          // of the file by clients only
          tcp_f.th_flags = TH_SYN;
          tcp_f.th_win = htons(284);
          tcp_f.th_sum = 0;
          tcp_f.th_urp = 0;
          uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&tcp_f));
          header.insert(header.end(), data,  data + sizeof(struct tcphdr));
        } else if (ncs.list[nhdr].compare("udp_t") == 0) {
          struct udphdr udp_f;
          udp_f.uh_sport = htons((uint16_t)packet_fields["sport"]);
          udp_f.uh_dport = htons((uint16_t)packet_fields["dport"]);
          udp_f.uh_ulen = 0;  // udp length
          udp_f.uh_sum = 0;  // udp checksum
          uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&udp_f));
          header.insert(header.end(), data, data + sizeof(struct udphdr));
        } else if (ncs.list[nhdr].compare("trackhdr") == 0) {
          struct trackhdr track_f;
          uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&track_f));
          header.insert(header.end(), data, data + sizeof(struct trackhdr));
        }
      }
      ncs.header_data.push_back(header);
    }
  } catch (std::exception &e) {
    std::cerr << "Exception while parsing client/server nodes!" << std::endl;
    assert(false);
  }
  return ncs;
}
ServerConfigStruct TestbedUtilities::getServerConfigurations
  (pfp::core::PFPObject* node, std::string configFile) {
  ServerConfigStruct ncs;
  try {
    json node_params = node->GetParameter("NODE").get();
    json instance_params = node->GetParameter("INSTANCE").get();
    json file_sizes = node->GetParameter("FILE_SIZES").get();
    json packet_fields = node->GetParameter("PACKET_FIELDS").get();
    json application_delays = node->GetParameter("APPLICATION_DELAYS").get();
    json ip_address = node_params["ip_address"];

    ncs.node_type = node_params["type"];
    if (ncs.node_type.compare("server") != 0) {
      std::cerr << "Incorrect config file " << configFile
        << " used for server configuration!" << std::endl;
      assert(false);
    }
    ncs.archive = node_params["archive"];

    auto headers_list = packet_fields["headers"];
    ncs.list.clear();
    for (std::string hdr : headers_list) {
      ncs.list.push_back(hdr);
    }
    if (ncs.list.size() == 0) {
      std::cerr << "No headers specified!" << std::endl;
      assert(false);
    }

    auto size_values = file_sizes["values"];
    std::string size_unit = file_sizes["unit"];
    for (int index = 0; index < size_values.size(); index++) {
      int32_t size_val = size_values.at(index);
      if (size_unit.compare("kB") == 0) {
        size_val *= 1024;
      } else if (size_unit.compare("MB") == 0) {
        size_val *= 1024;
        size_val *= 1024;
      } else if (size_unit.compare("GB") == 0) {
        size_val *= 1024;
        size_val *= 1024;
        size_val *= 1024;
      }
      ncs.fsize.size_values.push_back(size_val);
    }

    std::vector<std::string> mtu_vec = getStringVector(instance_params["mtu"]);
    ncs.mtu = stoi(mtu_vec.at(0));
    if (mtu_vec.at(1).compare("kB") == 0) {
      ncs.mtu *= 1024;
    } else if (mtu_vec.at(1).compare("MB") == 0) {
      ncs.mtu *= 1024;
      ncs.mtu *= 1024;
    } else if (mtu_vec.at(1).compare("GB") == 0) {
      ncs.mtu *= 1024;
      ncs.mtu *= 1024;
      ncs.mtu *= 1024;
    }
    std::vector<std::string> datarate_vec = getStringVector(
      instance_params["datarate"]);
    if (datarate_vec.size() > 0) {
      ncs.datarate = stol(datarate_vec.at(0));
      if (datarate_vec.at(1).compare("kbps") == 0) {
        ncs.datarate *= 1024;
      } else if (datarate_vec.at(1).compare("Mbps") == 0) {
        ncs.datarate *= 1024;
        ncs.datarate *= 1024;
      } else if (datarate_vec.at(1).compare("Gbps") == 0) {
        ncs.datarate *= 1024;
        ncs.datarate *= 1024;
        ncs.datarate *= 1024;
      }
    }

    auto ip_addr_pool_values = ip_address["pool"];
    ncs.prefixes.prefix_values.clear();
    for (std::string val : ip_addr_pool_values) {
      ncs.prefixes.prefix_values.push_back(val);
    }

    std::string ip_addr_policy = ip_address["policy"];
    std::vector<std::string> addr_policy_vec = getStringVector(ip_addr_policy);
    ncs.prefixes.distribution.type = addr_policy_vec.at(0);
    if (addr_policy_vec.size() < 2) {
      getDefaultDistributionParameters(ncs.prefixes.distribution.type,
          &ncs.prefixes.distribution.param1,
          &ncs.prefixes.distribution.param2);
    } else if (addr_policy_vec.size() < 3) {
      ncs.prefixes.distribution.param1 = stod(addr_policy_vec.at(1));
      ncs.prefixes.distribution.param2 = 0;
    } else {
      ncs.prefixes.distribution.param1 = stod(addr_policy_vec.at(1));
      ncs.prefixes.distribution.param2 = stod(addr_policy_vec.at(2));
    }

    std::string file_policy = file_sizes["policy"];
    std::vector<std::string> file_policy_vec = getStringVector(file_policy);
    ncs.fsize.distribution.type = file_policy_vec.at(0);
    if (file_policy_vec.size() < 2) {
      getDefaultDistributionParameters(ncs.fsize.distribution.type,
          &ncs.fsize.distribution.param1,
          &ncs.fsize.distribution.param2);
    } else if (file_policy_vec.size() < 3) {
      ncs.fsize.distribution.param1 = stod(file_policy_vec.at(1));
      ncs.fsize.distribution.param2 = 0;
    } else {
      ncs.fsize.distribution.param1 = stod(file_policy_vec.at(1));
      ncs.fsize.distribution.param2 = stod(file_policy_vec.at(2));
    }

    auto app_delay_values = application_delays["values"];
    std::string app_delay_unit = application_delays["unit"];
    for (int index = 0; index < app_delay_values.size(); index++) {
      if (app_delay_unit.compare("SEC") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_SEC));
      } else if (app_delay_unit.compare("MS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_MS));
      } else if (app_delay_unit.compare("US") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_US));
      } else if (app_delay_unit.compare("NS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_NS));
      } else if (app_delay_unit.compare("PS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_PS));
      } else if (app_delay_unit.compare("FS") == 0) {
        ncs.application_delay.delay_values.push_back(
            sc_time(app_delay_values.at(index), SC_FS));
      } else {
        std::cerr << "Server/ Client Unknown unit for delay specified! \n"
            << "Supported values are: SC_SEC, SC_MS, SC_US,"
            << "SC_NS, SC_PS, SC_FS" << std::endl;
        assert(false);
      }
    }
    auto ad_policy_config = getStringVector(application_delays["policy"]);
    ncs.application_delay.distribution.type = ad_policy_config.at(0);
    if (ad_policy_config.size() < 2) {
      getDefaultDistributionParameters(ncs.application_delay.distribution.type,
          &ncs.application_delay.distribution.param1,
          &ncs.application_delay.distribution.param2);
    } else if (ad_policy_config.size() < 3) {
      ncs.application_delay.distribution.param1 = stod(ad_policy_config.at(1));
      ncs.application_delay.distribution.param2 = 0;
    } else {
      ncs.application_delay.distribution.param1 = stod(ad_policy_config.at(1));
      ncs.application_delay.distribution.param2 = stod(ad_policy_config.at(2));
    }
  } catch (std::exception &e) {
    std::cerr << "Exception while parsing server nodes! Config file: "
      << configFile << std::endl;
    assert(false);
  }
  return ncs;
}

std::vector<std::string> TestbedUtilities::getStringVector
(const std::string &str) {
  std::vector<std::string> strVec;
  std::stringstream ss(str);
  std::istream_iterator<std::string> begin(ss);
  std::istream_iterator<std::string> end;
  strVec.insert(strVec.begin(), begin, end);
  return strVec;
}

void TestbedUtilities::getDefaultDistributionParameters
  (const std::string &distribution, double *param1, double *param2) {
  if (distribution.compare("binomial") == 0) {
    *param1 = 0.5;
    *param2 = 0;
  } else if (distribution.compare("geometric") == 0) {
    *param1 = 0.5;
    *param2 = 0;
  } else if (distribution.compare("poisson") == 0) {
    *param1 = 4;
    *param2 = 0;
  } else if (distribution.compare("exponential") == 0) {
    *param1 = 1;
    *param2 = 0;
  } else if (distribution.compare("weibull") == 0) {
    *param1 = 1;
    *param2 = 1;
  } else {
    *param1 = 0;
    *param2 = 0;
  }
}

int TestbedUtilities::getRandomNum
  (int min, int max, std::string distribution, double param1, double param2) {
  auto seed =
  std::chrono::high_resolution_clock::now().time_since_epoch().count();
  std::mt19937 rng(seed);
  int num = 0;
  if (distribution.compare("random") == 0) {
    /*
     * Generates random number between [0, 1)
     * 100 denotes a 100 bits of randomness
     * PS: We take anything above 0.999 to be 1
     */
    double temp = std::generate_canonical<double, 10>(rng);
    if (temp > 0.999) {
      temp = 1;
    }
    num = temp * max;
  } else if (distribution.compare("uniform") == 0) {
    /*
     * Generates uniformly distributed number between [mix, max]
     */
    std::uniform_int_distribution<> randomIndex(min, max);
    num = randomIndex(rng);
  } else if (distribution.compare("binomial") == 0) {
    /*
     * Generates a binomial distribution
     * No. of trials (t) = max number of headers
     * Probability of success = p (default is 0.5 for this application)
     */
    std::binomial_distribution<> randomIndex(max, param1);
    num = randomIndex(rng);
  } else if (distribution.compare("geometric") == 0) {
    /*
     * Generates a geometric distribution
     * if the generated value is less than min
     * or greater than max, we generate again
     */
    std::geometric_distribution<> randomIndex(param1);
    num = randomIndex(rng);
    while (num < min || num > max) {
      num = randomIndex(rng);
    }
  } else if (distribution.compare("poisson") == 0) {
    /*
     * Generates a poisson distribution
     * Mean is 4 by default
     */
    std::poisson_distribution<> randomIndex(param1);
    num = randomIndex(rng);
    while (num < min || num > max) {
      num = randomIndex(rng);
    }
  } else if (distribution.compare("exponential") == 0) {
    /*
     * Generates an exponential distribution
     * with rate of events lambda (def is 1)
     */
    std::exponential_distribution<> randomIndex(param1);
    num = static_cast<int>(randomIndex(rng));
    while (num < min || num > max) {
    }
      num = static_cast<int>(randomIndex(rng));
  } else if (distribution.compare("weibull") == 0) {
    /*
     * Generates an Weibull distribution
     * with rate of events lambda (def is 1)
     */
    std::weibull_distribution<> randomIndex(param1, param2);
    num = static_cast<int>(randomIndex(rng));
    while (num < min || num > max) {
      num = static_cast<int>(randomIndex(rng));
    }
  } else if (distribution.compare("normal") == 0) {
    /*
     * Generates an normal distribution
     * with mean as (max+min)/2
     * and standard deviation equal to mean
     * The distribution peaks at the center
     */
    double nor_mean = (max + min) / 2;
    double nor_stddev = nor_mean / 4;
    std::normal_distribution<> randomIndex(nor_mean, nor_stddev);
    num = static_cast<int>(randomIndex(rng));
    while (num < min || num > max) {
      num = static_cast<int>(randomIndex(rng));
    }
  } else if (distribution.compare("lognormal") == 0) {
    /*
     * Generates an lognormal distribution
     * with mean as (max+min)/2
     * and standard deviation equal to mean
     */
    double nor_mean = (max + min) / 2;
    double nor_stddev = nor_mean / 4;
    std::lognormal_distribution<> randomIndex(nor_mean, nor_stddev);
    num = static_cast<int>(randomIndex(rng));
    while (num < min || num > max) {
      num = static_cast<int>(randomIndex(rng));
    }
  }

  return num;
}

int TestbedUtilities::addHeaders(std::shared_ptr<TestbedPacket> pkt,
  const std::vector<uint8_t> &headers) {
  pkt->setData().insert(pkt->setData().begin(), headers.begin(), headers.end());
  return headers.size();
}

void TestbedUtilities::finalizePacket(std::shared_ptr<TestbedPacket> pkt,
  const std::vector<std::string> &headers) {
  /*
   * List of headers in configuration xml
   * 1. ether_header
   * 2. ip
   * 3. tcphdr
   * 4. trackhdr
   * 5. ip6_hdr
   * 6. udphdr
   */
  struct ether_header *ethernetptr;
  struct ip *ipptr;
  struct ip6_hdr *ip6ptr;
  struct tcphdr *tcpptr;
  struct udphdr *udpptr;
  struct trackhdr *trackptr;
  int loc = 0;
  std::vector<int> headerPos;
  /*
   * Get the locations of the various headers in the packet
   */
  for (std::string hdr : headers) {
    if (hdr.compare("ethernet_t") == 0) {
      headerPos.push_back(loc);
      loc += sizeof(struct ether_header);
    } else if (hdr.compare("ipv4_t") == 0) {
      headerPos.push_back(loc);
      loc += sizeof(struct ip);
    } else if (hdr.compare("tcp_t") == 0) {
      headerPos.push_back(loc);
      loc += sizeof(struct tcphdr);
    } else if (hdr.compare("udp_t") == 0) {
      headerPos.push_back(loc);
      loc += sizeof(struct udphdr);
    } else if (hdr.compare("trackhdr") == 0) {
      headerPos.push_back(loc);
      loc += sizeof(struct trackhdr);
    } else if (hdr.compare("ipv6_t") ==0) {
      headerPos.push_back(loc);
      loc += sizeof(struct ip6_hdr);
    }
    // else if (hdr.compare("<header_name>") == 0) {
    // <header_name>= dataptr+index;
    // hton<header_name>();
    // index += sizeof(<header_name>);
    // }
  }
  int headerIndex = headers.size() - 1;
  uint8_t *dataptr = pkt->setData().data();
  for (; headerIndex >= 0; headerIndex--) {
    std::string hdr = headers.at(headerIndex);
    if (hdr.compare("ethernet_t") == 0) {
      ethernetptr = (struct ether_header*) (dataptr + headerPos.back());
      headerPos.pop_back();
    } else if (hdr.compare("ipv4_t") == 0) {
      // extract header to be updated
      ipptr = (struct ip*) (dataptr + headerPos.back());
      // compute the checksum and packet length
      ipptr->ip_len = pkt->setData().size()
          - (sizeof(ip) + sizeof(struct ether_header));
      ipptr->ip_ttl = (uint8_t)64;  // ipv4_ttl;
      ipptr->ip_sum = calculateIPChecksum(ipptr, sizeof(struct ip));
      headerPos.pop_back();
    } else if (hdr.compare("ipv6_t") == 0) {
      // extract header to be updated
      ip6ptr = (struct ip6_hdr*) (dataptr + headerPos.back());
      ip6ptr->ip6_un1_plen = htonl(pkt->setData().size()
          - (sizeof(struct ip6_hdr) + sizeof(struct ether_header)));
      ip6ptr->ip6_un1_hlim = (uint8_t)64;
      headerPos.pop_back();
    } else if (hdr.compare("tcp_t") == 0) {
      // extract tcp and ip headers
      // only tcp header will be updated here
      // we need the IP header to fetch the IPs
      int tcpPos = headerPos.back();
      headerPos.pop_back();
      int ipPos = headerPos.back();
      tcpptr = (struct tcphdr*) (dataptr + tcpPos);
      ipptr = (struct ip*) (dataptr + ipPos);
      ipptr->ip_p = (uint8_t)6;
      tcpptr->th_sum = calculateTCPChecksum(pkt, tcpPos, 06, ipptr);
    } else if (hdr.compare("udp_t") == 0) {
      // extract header to be updated
      int udpPos = headerPos.back();
      headerPos.pop_back();
      int ipPos = headerPos.back();
      udpptr = (struct udphdr*) (dataptr + udpPos);
      ipptr = (struct ip*) (dataptr + ipPos);
      ipptr->ip_p = (uint8_t)17;
      udpptr->uh_ulen = htons(pkt->setData().size() - udpPos);
      // make sure checksum is always computed last
      udpptr->uh_sum = calculateTCPChecksum(pkt, udpPos, 17, ipptr);
    } else if (hdr.compare("trackhdr") == 0) {
      int trackPos = headerPos.back();
      trackptr = (struct trackhdr*) (dataptr + trackPos);
      trackptr->created = sc_time_stamp();
      trackptr->ip_src = ipptr->ip_src;
      trackptr->ip_dst = ipptr->ip_dst;
      for (int index = 0; index < 6; index++) {
        trackptr->ether_dhost[index] = ethernetptr->ether_dhost[index];
        trackptr->ether_shost[index] = ethernetptr->ether_shost[index];
      }
      trackptr->ttl = ipptr->ip_ttl;
      if (tcpptr != NULL) {
        trackptr->sport = tcpptr->th_sport;
        trackptr->dport = tcpptr->th_dport;
      } else  if (udpptr != NULL) {
        trackptr->sport = udpptr->uh_sport;
        trackptr->dport = udpptr->uh_dport;
      }
      headerPos.pop_back();
    }
    // else if (hdr.compare("<header_name>") == 0) {
    // <header_name>= dataptr+index;
    // if (doHTONS) {
    // htonheader_name();
    // }
    // headerPos.pop_back();
    // }
  }
}

uint16_t TestbedUtilities::calculateIPChecksum(void* vdata, int length) {
  uint8_t* data = reinterpret_cast<uint8_t*>(vdata);
  uint32_t acc = 0xffff;

  for (int i = 0; i + 1 < length; i += 2) {
    uint16_t word;
    memcpy(&word, data + i, 2);
    acc += word;
    if (acc > 0xffff) {
      acc -= 0xffff;
    }
  }

  if (length & 1) {
    uint16_t word = 0;
    memcpy(&word, data + length - 1, 1);
    acc += word;
    if (acc > 0xffff) {
      acc -= 0xffff;
    }
  }
  return ~(acc);
}

// Method which provides the checksum for UDP and TCP
uint16_t TestbedUtilities::calculateTCPChecksum
(std::shared_ptr<TestbedPacket> pkt, int hdrPos, uint8_t protocol
, struct ip *ipptr) {
  std::vector<uint8_t> cs_data;

  std::vector<uint8_t> pseudoheader;
  uint8_t rsrvd = (uint8_t) 0;
  uint16_t tcplen = htons((uint16_t) (pkt->setData().size() - hdrPos));
  uint8_t *temp_ptr =
      static_cast<uint8_t*>(static_cast<void *>(&ipptr->ip_src));
  pseudoheader.insert(pseudoheader.end(), temp_ptr,
      temp_ptr + sizeof(in_addr_t));
  temp_ptr = static_cast<uint8_t*>(static_cast<void *>(&ipptr->ip_dst));
  pseudoheader.insert(pseudoheader.end(), temp_ptr,
      temp_ptr + sizeof(in_addr_t));
  temp_ptr = static_cast<uint8_t*>(static_cast<void *>(&rsrvd));
  pseudoheader.insert(pseudoheader.end(), temp_ptr,
      temp_ptr + sizeof(uint8_t));
  temp_ptr = static_cast<uint8_t*>(static_cast<void *>(&protocol));
  pseudoheader.insert(pseudoheader.end(), temp_ptr,
      temp_ptr + sizeof(uint8_t));
  temp_ptr = static_cast<uint8_t*>(static_cast<void *>(&tcplen));
  pseudoheader.insert(pseudoheader.end(), temp_ptr,
      temp_ptr + sizeof(uint16_t));

  cs_data.insert(cs_data.begin(), pseudoheader.begin(), pseudoheader.end());
  cs_data.insert(cs_data.end(), pkt->setData().begin() +
    hdrPos, pkt->setData().end());

  uint16_t acc = calculateIPChecksum(cs_data.data(), cs_data.size());

  return acc;
}

/*
 Payload len in Bytes
 */
void TestbedUtilities::addPayload(std::shared_ptr<TestbedPacket> pkt, int len) {
  for (int index = 0; index < len; index++) {
    pkt->setData().push_back(0);
  }
}


void TestbedUtilities::getResponseHeader
(std::shared_ptr<TestbedPacket> reqPacket,
  std::shared_ptr<TestbedPacket> resPacket,
  int type, const std::vector<std::string> &headers) {
  struct ether_header *reqEth;
  struct ip *reqIP;
  struct tcphdr *reqTCP;
  struct udphdr *reqUDP;
  struct ether_header resEth;
  struct ip resIP;
  struct tcphdr resTCP;
  struct udphdr resUDP;
  //  Assuming Ethernet, IPv4 and TCP/ UDP headers
  reqEth = (struct ether_header*) reqPacket->setData().data();
  for (size_t index = 0; index < 6; index++) {
    resEth.ether_dhost[index] = reqEth->ether_shost[index];
    resEth.ether_shost[index] = reqEth->ether_dhost[index];
  }
  resEth.ether_type = reqEth->ether_type;
  uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&resEth));
  resPacket->setData().insert(resPacket->setData().end(), data,  data +
    sizeof(struct ether_header));

  reqIP =  (struct ip*) (reqPacket->setData().data() +
    sizeof(struct ether_header));
  resIP.ip_dst = reqIP->ip_src;
  if (type == 0) {
    resIP.ip_id = getRandomNum(0, INT_MAX, "random");
  }
  resIP.ip_len = 0;
  resIP.ip_off = reqIP->ip_off;
  resIP.ip_p = reqIP->ip_p;
  resIP.ip_src = reqIP->ip_dst;
  resIP.ip_sum = 0;
  resIP.ip_tos = reqIP->ip_tos;
  resIP.ip_ttl = (uint8_t)64;  // ipv4_ttl;
  resIP.ip_vhl = reqIP->ip_vhl;
  data =  static_cast<uint8_t*>(static_cast<void*>(&resIP));
  resPacket->setData().insert(resPacket->setData().end(), data,  data +
    sizeof(struct ip));


  if (headers.size() > 2 && headers.at(2).compare("tcp_t") == 0) {
    reqTCP = (struct tcphdr*) (reqPacket->setData().data() +
      sizeof(struct ether_header) + sizeof(struct ip));
    resTCP.th_dport = reqTCP->th_sport;
    resTCP.th_offx = reqTCP->th_offx;
    if (type == -1) {
      // Servers send a SYN/ACK packet
      resTCP.th_flags = TH_ACK |TH_SYN;
      resTCP.th_seq = getRandomNum(0, INT_MAX, "random");
      resTCP.th_ack = htonl(ntohl(reqTCP->th_seq) + ((type > 1) ? type : 1));
    } else if (type == -2) {
      // Servers send a ACK packet
      resTCP.th_flags = TH_ACK;
      resTCP.th_seq = reqTCP->th_ack;
      resTCP.th_ack = htonl(ntohl(reqTCP->th_seq) + ((type > 1) ? type : 1));
    } else if (type == -3) {
      // Servers send a PSH packet
      resTCP.th_flags =  TH_PUSH;
      resTCP.th_seq = reqTCP->th_ack;
      resTCP.th_ack = htonl(ntohl(reqTCP->th_seq) + ((type > 1) ? type : 1));
    } else if (type == -4) {
      // Servers send a PSH packet
      resTCP.th_flags = TH_ACK | TH_FIN;
      resTCP.th_seq = reqTCP->th_ack;
      resTCP.th_ack = htonl(ntohl(reqTCP->th_seq) + ((type > 1) ? type : 1));
    } else if (type == -5) {
      // Clients send a RST packet
      resTCP.th_flags = TH_RST;
      resTCP.th_seq = reqTCP->th_ack;
      resTCP.th_ack = htonl(ntohl(reqTCP->th_seq) + ((type > 1) ? type : 1));
    } else if (type >= 0) {
      // Servers send a ACK pakcet for a TCP payload of size N
      // Typically we send a ACK packet as a piggyback only
      resTCP.th_flags = TH_ACK | TH_PUSH;
      resTCP.th_seq = reqTCP->th_ack;
      resTCP.th_ack = htonl(ntohl(reqTCP->th_seq) + type);
    }
    resTCP.th_sport = reqTCP->th_dport;
    resTCP.th_sum = 0;
    resTCP.th_urp = 0;
    resTCP.th_win = reqTCP->th_win;
    data =  static_cast<uint8_t*>(static_cast<void*>(&resTCP));
    resPacket->setData().insert(resPacket->setData().end(), data,  data +
      sizeof(struct tcphdr));
  } else if (headers.size() > 2 && headers.at(2).compare("udp_t") == 0) {
    reqUDP = (struct udphdr*) (reqPacket->setData().data() +
      sizeof(struct ether_header) + sizeof(struct ip));
    resUDP.uh_sport = reqUDP->uh_sport;
    resUDP.uh_dport = reqUDP->uh_dport;
    resUDP.uh_ulen = 0;
    resUDP.uh_sum = 0;
    data =  static_cast<uint8_t*>(static_cast<void*>(&resUDP));
    resPacket->setData().insert(resPacket->setData().end(),
      data,  data + sizeof(struct udphdr));
  }
  if (headers.size() > 3 && headers.at(3).compare("dns_t") == 0) {
    struct dnshdr *reqDNS;
    struct dnshdr resDNS;
    reqDNS = (struct dnshdr*)(reqPacket->setData().data() +
      sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct udphdr));
    resDNS = *reqDNS;
    resDNS.qr = 1;
    resDNS.q_count = 0;
    resDNS.ans_count = htons(static_cast<uint16_t>(1));
    data =  static_cast<uint8_t*>(static_cast<void*>(&resDNS));
    resPacket->setData().insert(resPacket->setData().end(), data,
      data + sizeof(struct dnshdr));
    int questionPos = sizeof(struct ether_header) + sizeof(struct ip)
      + sizeof(struct udphdr) +sizeof(struct dnshdr);
    // Copy the question part as well as that is part of the answer also :)
    resPacket->setData().insert(resPacket->setData().end(),
    reqPacket->setData().begin() + questionPos, reqPacket->setData().end());
  }
  // struct trackhdr resTrack;
}

std::vector<std::string> TestbedUtilities::getIPv4List(std::string dhcpPool,
  int maxListSize, int initIndex) {
  std::vector<std::string> ipv4List;
  std::stringstream dhcpPoolss(dhcpPool);
  std::string mask, prefix;
  std::getline(dhcpPoolss, mask, '/');
  std::getline(dhcpPoolss, prefix, '/');
  std::bitset<32> andMask, orMask;
  std::bitset<32> maskbits, minIP, maxIP;
  int bitsav = 32-stoi(prefix);
  ipv4List.clear();
  if (bitsav == 0) {
    ipv4List.push_back(mask);
  } else {
    andMask = std::bitset<32>().set();
    for (int index = 0; index < bitsav; index++) {
      andMask[index].flip();
    }
    orMask = ~andMask;
    struct in_addr temp;
    if (inet_aton(mask.c_str(), &temp) == 0) {
      assert(!"Incorrect IP received");
    }

    maskbits = std::bitset<32>(ntohl(temp.s_addr));
    minIP = std::bitset<32>(maskbits & andMask);
    uint32_t minIPlong = minIP.to_ulong();

    maxIP = std::bitset<32>(maskbits | orMask);
    uint32_t maxIPlong = maxIP.to_ulong();
    temp.s_addr = htonl(minIPlong);
    temp.s_addr = htonl(maxIPlong);
    minIPlong = minIPlong + (uint32_t)initIndex;
    temp.s_addr = htonl(minIPlong);
    while (minIPlong <= maxIPlong) {
      temp.s_addr = htonl(minIPlong);
      if (inet_ntoa(temp) == 0) {
        assert(!"Incorrect IP received");
      }
      std::string memip = inet_ntoa(temp);
      minIPlong++;
      ipv4List.push_back(memip);
      if (ipv4List.size() == maxListSize) {
        break;
      }
    }
  }
  return ipv4List;
}

std::vector<std::string> TestbedUtilities::interleaveVectors
  (const std::vector<std::vector<std::string> >& cps2) {
  std::vector<std::string> cps;
  int maxVectorSize = 0;
  for (int tvindex = 0; tvindex < cps2.size(); tvindex++) {
    if (maxVectorSize < cps2.at(tvindex).size()) {
      maxVectorSize = cps2.at(tvindex).size();
    }
  }
  for (int ind = 0; ind < maxVectorSize; ind++) {
    for (std::vector<std::string> vec : cps2) {
      if (vec.size() > ind) {
        cps.push_back(vec.at(ind));
      }
    }
  }
  return cps;
}

std::string TestbedUtilities::getIPAddress(
  const std::vector<uint8_t> &packet, const std::vector<std::string> &list,
  const std::string &type) {
  struct ip *ipptr;
  std::string id;
  for (std::string header_name : list) {
    if (header_name.compare("ipv4_t")) {
      ipptr = (struct ip*)(packet.data() + sizeof(ether_header));
      if (type.compare("src") == 0) {
        if (inet_ntoa(ipptr->ip_src) == 0) {
          assert(!"Incorrect ip_src");
        }
        id = inet_ntoa(ipptr->ip_src);
      } else {
        if (inet_ntoa(ipptr->ip_dst) == 0) {
          assert(!"Incorrect ip_dst");
        }
        id = inet_ntoa(ipptr->ip_dst);
      }
    }
  }
  return id;
}

size_t TestbedUtilities::getHeaderLength(
  const std::vector<std::string> &headers) {
  size_t headerLen = 0;
  for (std::string hdr : headers) {
    if (hdr.compare("ethernet_t") == 0) {
      headerLen += sizeof(struct ether_header);
    } else if (hdr.compare("ipv4_t") == 0) {
      headerLen += sizeof(struct ip);
    } else if (hdr.compare("tcp_t") == 0) {
      headerLen += sizeof(struct tcphdr);
    } else if (hdr.compare("udp_t") == 0) {
      headerLen += sizeof(struct udphdr);
    } else if (hdr.compare("trackhdr") == 0) {
      headerLen += sizeof(struct trackhdr);
    } else if (hdr.compare("ipv6_t") ==0) {
      headerLen += sizeof(struct ip6_hdr);
    } else if (hdr.compare("dns_t") ==0) {
      headerLen += sizeof(struct dnshdr);
    }
  }
  return headerLen;
}

std::vector<uint8_t> TestbedUtilities::getLayer4Header(
  const std::vector<uint8_t> &packet, const std::vector<std::string> &headers) {
  size_t headerPos = 0;
  std::vector<uint8_t> l4hdr;
  for (std::string hdr : headers) {
    if (hdr.compare("ethernet_t") == 0) {
      headerPos += sizeof(struct ether_header);
    } else if (hdr.compare("ipv4_t") == 0) {
      headerPos += sizeof(struct ip);
    } else if (hdr.compare("tcp_t") == 0) {
      l4hdr.insert(l4hdr.begin(), packet.data() + headerPos,
        packet.data() + headerPos + sizeof(tcphdr));
      break;
    } else if (hdr.compare("udp_t") == 0) {
      l4hdr.insert(l4hdr.begin(), packet.data() + headerPos,
        packet.data() + headerPos + sizeof(udphdr));
      break;
    } else if (hdr.compare("trackhdr") == 0) {
      headerPos += sizeof(struct trackhdr);
    } else if (hdr.compare("ipv6_t") ==0) {
      headerPos += sizeof(struct ip6_hdr);
    }
  }
  return l4hdr;
}

std::string TestbedUtilities::getServerInstanceAddress(const AddrType &addrType,
  const std::map<std::string, size_t> &sessions, int initIndex) {
    size_t prefixIndex = -1;
    int elements = 10;
    size_t prefixLen = addrType.prefix_values.size();
    if (addrType.distribution.type.compare("round_robin") == 0) {
      // loop to go over the prefixes one by one
      // Everytime we get elements number of IPs
      // then we check what we can assign first :)
      while (true) {
        int maxIPs = 0;
        std::vector<std::vector<std::string> > allIPS;
        for (std::string dhcpPool : addrType.prefix_values) {
          std::vector<std::string> msqips = getIPv4List(dhcpPool, elements,
            initIndex);
          allIPS.push_back(msqips);
          if (maxIPs < msqips.size()) {
            maxIPs = msqips.size();
          }
        }
        for (int i = 0; i < elements; i++) {
          for (int j = 0; j < prefixLen; j++) {
            std::string sid = allIPS.at(j).at(i);
            if (sessions.find(sid) == sessions.end()) {
              return sid;
            }
          }
        }
        if (maxIPs < elements) {
          assert(!"New server instance could not be added! Possibly ran out of"
            + " assignable IP addresses!");
        }
        elements *= 2;
      }
    } else {
      prefixIndex = getRandomNum(0, prefixLen - 1,
        addrType.distribution.type, addrType.distribution.param1,
        addrType.distribution.param2);
    }
    std::string prefix = addrType.prefix_values.at(prefixIndex);
    // std::string sid;
    elements = 10;
    while (true) {
      std::vector<std::string> ips = getIPv4List(prefix, elements, initIndex);
      for (std::string sid : ips) {
        if (sessions.find(sid) == sessions.end()) {
          return sid;
        }
      }
      if (ips.size() < elements) {
        assert(!"New server instance could not be added! Possibly ran out of" +
          " assignable IP addresses!");
      }
      elements *= 2;
    }

    return NULL;
}
std::vector<std::string> TestbedUtilities::getBaseIPs(
  const std::vector<std::string> &prefixes) {
  std::vector<std::string> baseIPs;
  for (std::string prefix : prefixes) {
    std::vector<std::string> msqips = getIPv4List(prefix, 1, 0);
    baseIPs.push_back(msqips.at(0));
  }
  return baseIPs;
}
void TestbedUtilities::updateAddress(std::shared_ptr<TestbedPacket> pkt,
  const std::vector<std::string> &headers, const std::string &newAddress,
  const std::string &type) {
  size_t headerPos = 0;
  for (std::string hdr : headers) {
    if (hdr.compare("ethernet_t") == 0) {
      headerPos += sizeof(struct ether_header);
    } else if (hdr.compare("ipv4_t") == 0) {
      struct ip* ipptr = (struct ip*)(pkt->setData().data() + headerPos);
      if (type.compare("dst") == 0) {
        if (inet_aton(newAddress.c_str(), &ipptr->ip_dst) == 0) {
          std::cerr << "We got an invalid IP address - 03! Server addr: "
            << newAddress << std::endl;
          assert(false);
        }
      } else if (type.compare("src") == 0) {
        if (inet_aton(newAddress.c_str(), &ipptr->ip_src) == 0) {
          std::cerr << "We got an invalid IP address - 04! Server addr: "
            << newAddress << std::endl;
          assert(false);
        }
      }
    } else if (hdr.compare("ipv6_t") ==0) {
      headerPos += sizeof(struct ip6_hdr);
    }
  }
}
// All our DNS Packets will be UDP packets
// 1. type = 0 : DNS Query
// 2. type = 1 : DNS Response
void TestbedUtilities::getDnsPacket(std::shared_ptr<TestbedPacket> reqPacket,
  std::shared_ptr<TestbedPacket> resPacket, int type,
  const std::vector<std::string> &headers, const std::string &message) {
  struct tcphdr *tcpptr;
  // create a new packet
  if (type == 0) {
    int headerPos = 0;
    resPacket->setData().clear();
    for (std::string hdr : headers) {
      if (hdr.compare("ethernet_t") == 0) {
        struct ether_header *ethernetptr;
        ethernetptr = (struct ether_header*)(reqPacket->getData().data());
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(ethernetptr));
        resPacket->setData().insert(resPacket->setData().end(), data,
          data + sizeof(struct ether_header));
        headerPos += sizeof(struct ether_header);
      } else if (hdr.compare("ipv4_t") == 0) {
        struct ip *ipptr;
        ipptr = (struct ip*)(reqPacket->getData().data() + headerPos);
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(ipptr));
        resPacket->setData().insert(resPacket->setData().end(), data,
          data + sizeof(struct ip));
        headerPos += sizeof(struct ip);
      } else if (hdr.compare("ipv6_t") ==0) {
        struct ip6_hdr *ip6ptr;
        ip6ptr = (struct ip6_hdr*)(reqPacket->getData().data() + headerPos);
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(ip6ptr));
        resPacket->setData().insert(resPacket->setData().end(), data,
          data + sizeof(struct ip6_hdr));
        headerPos += sizeof(struct ip6_hdr);
      } else if (hdr.compare("udp_t") == 0) {
        struct udphdr *udpptr;
        udpptr = (struct udphdr*)(reqPacket->getData().data() + headerPos);
        udpptr->uh_dport = (htons)(53);
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(udpptr));
        resPacket->setData().insert(resPacket->setData().end(), data,
          data + sizeof(struct udphdr));
        headerPos += sizeof(struct udphdr);
      } else if (hdr.compare("tcp_t") == 0) {
        tcpptr = (struct tcphdr*)(reqPacket->getData().data() + headerPos);
        struct udphdr udpheader;
        udpheader.uh_dport = (htons)(53);  // tcpptr->th_dport;
        udpheader.uh_sport = tcpptr->th_sport;
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&udpheader));
        resPacket->setData().insert(resPacket->setData().end(), data,
          data + sizeof(struct udphdr));
        headerPos += sizeof(struct udphdr);
      }
    }
    struct dnshdr dnsheader;
    dnsheader.id = getRandomNum(0, 100000, "uniform");  // anything random
    dnsheader.qr = 0;  // 0 query, 1 response
    dnsheader.opcode = 0;  // 0 standard query
    dnsheader.aa = 0;  // authoritive answer
    dnsheader.tc = 0;  // truncated message
    dnsheader.rd = 0;  // recursion desired
    dnsheader.ra = 0;  // recursion available
    dnsheader.z = 0;  // reserved
    dnsheader.rcode = 0;  // response code
    dnsheader.q_count = htons(static_cast<uint16_t>(1));
    dnsheader.ans_count = 0;
    dnsheader.auth_count = 0;
    dnsheader.add_count = 0;

    uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&dnsheader));
    resPacket->setData().insert(resPacket->setData().end(), data,
      data + sizeof(struct dnshdr));

    // Now add the question
    std::stringstream ss(message);
    std::string item;
    std::vector<uint8_t> question;

    while (std::getline(ss, item, '.')) {
      int x = item.size() - 0;
      question.push_back((uint8_t)x);
      question.insert(question.end(), item.c_str(), item.c_str() + x);
    }
    question.push_back((uint8_t)0);
    while (question.size() %4 != 0) {
      question.push_back((uint8_t)0);
    }
    uint16_t temp = htons((uint16_t)(1));
    data = static_cast<uint8_t*>(static_cast<void*>(&temp));
    // Once for the Qtype
    question.insert(question.end(), data, data + 2);
    // Once for the Qclass
    question.insert(question.end(), data, data + 2);

    resPacket->setData().insert(resPacket->setData().end(), question.begin(),
      question.end());
  } else {
    // Create the DNS response packet
    std::vector<std::string> dnsHeaderList;
    for (std::string temp : headers) {
      if (temp.compare("ethernet_t") == 0) {
        dnsHeaderList.push_back("ethernet_t");
      } else if (temp.compare("ipv4_t") == 0) {
        dnsHeaderList.push_back("ipv4_t");
      } else if (temp.compare("ipv6_t") == 0) {
        dnsHeaderList.push_back("ipv6_t");
      } else if (temp.compare("udp_t") == 0) {
        dnsHeaderList.push_back("udp_t");
      } else if (temp.compare("tcp_t") == 0) {
        dnsHeaderList.push_back("udp_t");
      }
    }
    dnsHeaderList.push_back("dns_t");
    getResponseHeader(reqPacket, resPacket, type, dnsHeaderList);

    // Add the answer part
    uint32_t ttl = 0;
    uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&ttl));
    resPacket->setData().insert(resPacket->setData().end(), data, data + 4);
    uint16_t rdlen = htons((uint16_t)(4));
    data =  static_cast<uint8_t*>(static_cast<void*>(&rdlen));
    resPacket->setData().insert(resPacket->setData().end(), data, data + 2);

    struct in_addr dnsreply;
    if (inet_aton(message.c_str(), &dnsreply) == 0) {
      std::cerr << "We got an invalid DNS reply: " << message
          << std::endl;
      assert(false);
    }
    data =  static_cast<uint8_t*>(static_cast<void*>(&dnsreply));
    resPacket->setData().insert(resPacket->setData().end(), data,
      data + sizeof(struct in_addr));
  }
}
std::string  TestbedUtilities::getDNSResponseIPAddr(
  std::shared_ptr<TestbedPacket> pkt, const std::vector<std::string> &headers) {
  // Query starts from here.. actually answer is only the last 4 octets
  struct in_addr *dnsreply = (struct in_addr*)(pkt->getData().data()
    + pkt->getData().size() - sizeof(struct in_addr));
  if (inet_ntoa(*dnsreply) == 0) {
    assert(!"Incorrect dns reply received!");
  }
  std::string rep = inet_ntoa(*dnsreply);
  return rep;
}
std::string  getDNSRequestURL(std::shared_ptr<TestbedPacket> pkt,
    const std::vector<std::string> &headers) {
}
void TestbedUtilities::dissectPacket(const std::vector<uint8_t> &packet_data,
  const std::vector<std::string> &headers) {
  size_t headerPos = 0;
  for (std::string hdr : headers) {
    if (hdr.compare("ethernet_t") == 0) {
      struct ether_header *ethernetptr
        = (struct ether_header*)(packet_data.data() + headerPos);
      std::cout << "ether_dhost: ";
      for (int index = 0; index < 6; index++) {
        std::cout << (uint32_t)ethernetptr->ether_dhost[index] << ":";
      } std::cout << std::endl;
      std::cout << "ether_shost: ";
      for (int index = 0; index < 6; index++) {
        std::cout << (uint32_t)ethernetptr->ether_shost[index] << ":";
      } std::cout << std::endl;
      std::cout << "ether_type: " << (uint32_t)ntohs(ethernetptr->ether_type)
        << std::endl;
      headerPos += sizeof(struct ether_header);
    } else if (hdr.compare("ipv4_t") == 0) {
      struct ip *ipptr
        = (struct ip*)(packet_data.data() + headerPos);
      std::cout << "ip_vhl" << ipptr->ip_vhl  << std::endl;
      std::cout << "ip_tos" << ipptr->ip_tos  << std::endl;
      std::cout << "ip_len" << ntohs(ipptr->ip_len)     << std::endl;
      std::cout << "ip_id"  << ntohs(ipptr->ip_id)      << std::endl;
      std::cout << "ip_off" << ntohs(ipptr->ip_off)     << std::endl;
      std::cout << "ip_ttl" << ipptr->ip_ttl  << std::endl;
      std::cout << "ip_p"   << ipptr->ip_p    << std::endl;
      std::cout << "ip_sum" << ntohs(ipptr->ip_sum)     << std::endl;
      if (inet_ntoa(ipptr->ip_src) == 0) {
        assert(!"Incorrect IP received");
      }
      std::cout << "ip_src" << inet_ntoa(ipptr->ip_src) << std::endl;
      if (inet_ntoa(ipptr->ip_dst) == 0) {
        assert(!"Incorrect IP received");
      }
      std::cout << "ip_dst" << inet_ntoa(ipptr->ip_dst) << std::endl;
      headerPos += sizeof(struct ip);
    } else if (hdr.compare("tcp_t") == 0) {
      struct tcphdr *tcpptr
        = (struct tcphdr*)(packet_data.data() + headerPos);
      std::cout << "th_sport" << ntohs(tcpptr->th_sport)   << std::endl;
      std::cout << "th_dport" << ntohs(tcpptr->th_dport)   << std::endl;
      std::cout << "th_seq"   << ntohl(tcpptr->th_seq)     << std::endl;
      std::cout << "th_ack"   << ntohl(tcpptr->th_ack)     << std::endl;
      std::cout << "th_offx"  << (uint32_t)tcpptr->th_offx << std::endl;
      std::cout << "th_flags" << (uint32_t)tcpptr->th_flags << std::endl;
      std::cout << "th_win"   << ntohs(tcpptr->th_win)      << std::endl;
      std::cout << "th_sum"   << ntohs(tcpptr->th_sum)      << std::endl;
      std::cout << "th_urp"   << ntohs(tcpptr->th_urp)      << std::endl;
      headerPos += sizeof(struct tcphdr);
    } else if (hdr.compare("udp_t") == 0) {
      struct udphdr *udpptr
        = (struct udphdr*)(packet_data.data() + headerPos);
      std::cout << "uh_sport" << ntohs(udpptr->uh_sport) << std::endl;
      std::cout << "uh_dport" << ntohs(udpptr->uh_dport) << std::endl;
      std::cout << "uh_ulen"  << ntohs(udpptr->uh_ulen)  << std::endl;
      std::cout << "uh_sum"   << ntohs(udpptr->uh_sum)   << std::endl;
      headerPos += sizeof(struct udphdr);
    }
  }
}

void TestbedUtilities::getLoadBalancerPacket(
  std::shared_ptr<TestbedPacket> lb_packet,
  const std::map<std::string, size_t> &server_sessions,
  const std::string &node_id,
  const std::vector<std::string> &prefixes,
  const std::string &controller_ip,
  std::shared_ptr<TestbedPacket> received_packet,
  const std::vector<std::string> &headers) {
  // To create the load balancer packet, we find the response packet for the
  // received packet. Next, we assign the destination IP for the lb_packet to be
  // the controller_ip. We also update the tcp_t header, if present, to the
  // udp_t header.
  // The payload for the udp header is the server_sessions manager pairs
  // the format of the payload is:
  // server_node id + number of instances + (instance id + instance load)
  // ____________________
  // |    ETHER-II      |
  // |    IPv4          |
  // |    UDP           |
  // |------------------|
  // |    URL           |
  // |    prefix_count  |
  // |   *[prefix       |
  // |     length]      |
  // |   instance_count |
  // |   *[instances    |
  // |     load]        |
  // --------------------
  // struct tcphdr *tcpptr;
  lb_packet->setData().clear();
  // if (received_packet == NULL) {
    // This is generally the case when the server is initializing...
    // We create a ethernet-ipv4-udp packet with the load balancer info as
    // payload
    struct ether_header eth_f;
    // ("ipv4_t") == 0) {
    eth_f.ether_type = htons(static_cast<uint16_t>(2048));
    // ("ipv6_t") == 0) {
    //  eth_f.ether_type = 0x86DD;
    // FOR NOW LEFT DEST MAC AND SOURCE MAC AS GARBAGE
    uint8_t *data = static_cast<uint8_t*>(static_cast<void*>(&eth_f));
    lb_packet->setData().insert(lb_packet->setData().begin(), data,
        data + sizeof(struct ether_header));
    // (ncs.list[nhdr].compare("ipv4_t") == 0) {
    struct ip ip_f;
    uint8_t vhl = 69;
    ip_f.ip_vhl = vhl;
    int tos = 0;
    ip_f.ip_tos = (uint8_t) tos;
    uint16_t len = 16;
    ip_f.ip_len = htons(len);
    ip_f.ip_id = htons(static_cast<int16_t>(getRandomNum(
      0, SHRT_MAX, "uniform")));
    ip_f.ip_off = 0;
    int ttl = 64;  // stoi(configMap.find("ttl")->second);
    ip_f.ip_ttl = (uint8_t) ttl;
    // ipv4_ttl = ip_f.ip_ttl;
    // compare("tcp_t") == 0) {
    //  uint8_t proct = 6;
    //  ip_f.ip_p = proct;
    // compare("udp_t") == 0) {
    uint8_t proct = 17;
    ip_f.ip_p = proct;
    ip_f.ip_sum = 0;

    std::string ipsrc = server_sessions.begin()->first;  // , ipdst;
    // serverIPs, clientIPs
    // ipdst = serverIPs.at(index);
    // dipsrc = clientIPs.at(index);

    if (inet_aton(controller_ip.c_str(), &ip_f.ip_dst) == 0) {
      std::cerr << "We got an invalid IP address - 08! Server addr: "
        << controller_ip << std::endl;
      assert(false);
    }
    if (inet_aton(ipsrc.c_str(), &ip_f.ip_src) == 0) {
      std::cerr << "We got an invalid IP address - 09! Src: " << ipsrc
          << std::endl;
      assert(false);
    }
    data =  static_cast<uint8_t*>(static_cast<void*>(&ip_f));
    lb_packet->setData().insert(lb_packet->setData().end(), data,
      data + sizeof(struct ip));
    // else if (ncs.list[nhdr].compare("udp_t") == 0) {
    struct udphdr udp_f;
    // std::string strport = configMap.find("sport")->second;
    // uint16_t port = (uint16_t) stoi(strport);
    udp_f.uh_sport =  htons(static_cast<int16_t>(getRandomNum(0, SHRT_MAX,
      "uniform")));
    // strport = configMap.find("dport")->second;
    // port = (uint16_t) stoi(strport);
    udp_f.uh_dport = htons(static_cast<int16_t>(getRandomNum(0, SHRT_MAX,
      "uniform")));
    udp_f.uh_ulen = 0; /* udp length */
    udp_f.uh_sum = 0; /* udp checksum */
    data =  static_cast<uint8_t*>(static_cast<void*>(&udp_f));
    lb_packet->setData().insert(lb_packet->setData().end(), data,
      data + sizeof(struct udphdr));
  /*} else {
    std::shared_ptr<TestbedPacket> response_packet =
      std::make_shared<TestbedPacket>();
    getResponseHeader(received_packet, response_packet, 0, headers);

    int headerPos = 0;
    lb_packet->setData().clear();
    for (std::string hdr : headers) {
      if (hdr.compare("ethernet_t") == 0) {
        struct ether_header *ethernetptr;
        ethernetptr = (struct ether_header*)(response_packet->getData().data());
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(ethernetptr));
        lb_packet->setData().insert(lb_packet->setData().end(), data,
          data + sizeof(struct ether_header));
        headerPos += sizeof(struct ether_header);
      } else if (hdr.compare("ipv4_t") == 0) {
        struct ip *ipptr;
        ipptr = (struct ip*)(response_packet->getData().data() + headerPos);
        if (inet_aton(controller_ip.c_str(), &ipptr->ip_dst) == 0) {
          std::cerr << "We got an invalid IP address - 05! Controller addr: "
            << controller_ip << std::endl;
          assert(false);
        }
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(ipptr));
        lb_packet->setData().insert(lb_packet->setData().end(), data,
          data + sizeof(struct ip));
        headerPos += sizeof(struct ip);
      } else if (hdr.compare("ipv6_t") ==0) {
        struct ip6_hdr *ip6ptr;
        ip6ptr = (struct ip6_hdr*)(response_packet->getData().data() +
          headerPos);
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(ip6ptr));
        lb_packet->setData().insert(lb_packet->setData().end(), data,
          data + sizeof(struct ip6_hdr));
        headerPos += sizeof(struct ip6_hdr);
      } else if (hdr.compare("udp_t") == 0) {
        struct udphdr *udpptr;
        udpptr = (struct udphdr*)(response_packet->getData().data() +
          headerPos);
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(udpptr));
        lb_packet->setData().insert(lb_packet->setData().end(), data,
          data + sizeof(struct udphdr));
        headerPos += sizeof(struct udphdr);
      } else if (hdr.compare("tcp_t") == 0) {
        tcpptr = (struct tcphdr*)(response_packet->getData().data() +
          headerPos);
        struct udphdr udpheader;
        udpheader.uh_dport = tcpptr->th_dport;
        udpheader.uh_sport = tcpptr->th_sport;
        uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&udpheader));
        lb_packet->setData().insert(lb_packet->setData().end(), data,
          data + sizeof(struct udphdr));
        headerPos += sizeof(struct udphdr);
        // resPacket->setData().end());
      }
    }
  }*/
  // Time for the payload
  // Adding node id
  // We add it as we add it for DNS question :)
  // Let's follow a standard ;)
  std::stringstream ss(node_id);
  std::string item;
  std::vector<uint8_t> parsed_node_id;

  while (std::getline(ss, item, '.')) {
    int x = item.size() - 0;
    parsed_node_id.push_back((uint8_t)x);
    parsed_node_id.insert(parsed_node_id.end(), item.c_str(), item.c_str() + x);
  }
  parsed_node_id.push_back((uint8_t)0);
  while (parsed_node_id.size() %4 != 0) {
    parsed_node_id.push_back((uint8_t)0);
  }
  lb_packet->setData().insert(lb_packet->setData().end(),
    parsed_node_id.begin(), parsed_node_id.end());

  // Adding the prefix count
  uint32_t prefix_count = htonl((uint32_t)prefixes.size());
  // uint8_t *
    data = static_cast<uint8_t*>(static_cast<void*>(&prefix_count));
  lb_packet->setData().insert(lb_packet->setData().end(), data,
    data + sizeof(uint32_t));
  // Adding prefixes (prefix ip, prefix length[32])
  for (std::string tpre : prefixes) {
    std::stringstream prefixipss(tpre);
    std::string mask, prefix;
    std::getline(prefixipss, mask, '/');
    std::getline(prefixipss, prefix, '/');
    struct in_addr tempip;
    if (inet_aton(mask.c_str(), &tempip) == 0) {
      std::cerr << "We got an invalid IP address - 07! instance_id: "
        << mask << std::endl;
      assert(false);
    }
    data = static_cast<uint8_t*>(static_cast<void*>(&tempip));
    lb_packet->setData().insert(lb_packet->setData().end(), data,
      data + sizeof(struct in_addr));
    uint32_t mask_int;
    try {
      mask_int = htonl((uint32_t)stol(prefix));
    } catch (std::exception &e) {
      assert(!"Incorrect mask received");
    }
    data = static_cast<uint8_t*>(static_cast<void*>(&mask_int));
    lb_packet->setData().insert(lb_packet->setData().end(), data,
      data + sizeof(uint32_t));
  }

  // Adding instance counts
  uint32_t instance_count = htonl((uint32_t)server_sessions.size());
  data = static_cast<uint8_t*>(static_cast<void*>(&instance_count));
  lb_packet->setData().insert(lb_packet->setData().end(), data,
    data + sizeof(uint32_t));
  // Adding instances and load
  struct in_addr tempip;
  for (std::map<std::string, size_t>::const_iterator it =
    server_sessions.begin(); it != server_sessions.end(); ++it) {
    if (inet_aton(it->first.c_str(), &tempip) == 0) {
      std::cerr << "We got an invalid IP address - 06! instance_id: "
        << it->first << std::endl;
      assert(false);
    }
    data = static_cast<uint8_t*>(static_cast<void*>(&tempip));
    lb_packet->setData().insert(lb_packet->setData().end(), data,
      data + sizeof(struct in_addr));
    uint32_t server_load = htonl((uint32_t)it->second);
    data = static_cast<uint8_t*>(static_cast<void*>(&server_load));
    lb_packet->setData().insert(lb_packet->setData().end(), data,
      data + sizeof(uint32_t));
  }
}
std::vector<std::string> TestbedUtilities::getPacketHeaders(
  const std::vector<uint8_t> &pkt) {
  size_t headerpos = 0;
  std::vector<std::string> headers;
  struct ether_header *etherptr;
  // We always assume first header to be ethernet header
  etherptr = (struct ether_header*)(pkt.data());
  headers.push_back("ethernet_t");
  headerpos += sizeof(struct ether_header);
  // if (ncs.list.size() >= 2 &&
  //    ncs.list[nhdr + 1].compare("ipv4_t") == 0) {
  //  eth_f.ether_type = ntohs(2048);
  // } else if (ncs.list.size() >= 2 &&
  //    ncs.list[nhdr + 1].compare("ipv6_t") == 0) {
  //  eth_f.ether_type = 0x86DD;
  // (uint32_t)ntohs(ethernetptr->ether_type)
  uint32_t etype = (uint32_t)ntohs(etherptr->ether_type);
  // cout << etype << endl;
  if (etype == 2048) {
    struct ip *ipptr = (struct ip*)(pkt.data() +  headerpos);
    headers.push_back("ipv4_t");
    headerpos += sizeof(struct ip);
    uint16_t ip_protocol = (uint16_t)ipptr->ip_p;
    if (ip_protocol == 6) {
      struct tcphdr *tcpptr = (struct tcphdr*)(pkt.data() +  headerpos);
      headers.push_back("tcp_t");
      if (tcpptr->th_dport == htons(53)) {
        std::cerr << "We do not support DNS over TCP yet!" << endl;
        assert(false);
      }
    } else if (ip_protocol == 17) {
      struct udphdr *udpptr = (struct udphdr*)(pkt.data() +  headerpos);
      headers.push_back("udp_t");
      if (udpptr->uh_dport == htons(53)) {
        headers.push_back("dns_t");
      }
    }
  } else if (etherptr->ether_type == 0x86DD) {
    struct ip6_hdr *ipptr = (struct ip6_hdr*)(pkt.data() +  headerpos);
    headers.push_back("ipv6_t");
    headerpos += sizeof(struct ip6_hdr);
    uint16_t ip_protocol = (uint16_t)ipptr->ip6_un1_nxt;
    if (ip_protocol == 6) {
      struct tcphdr *tcpptr = (struct tcphdr*)(pkt.data() +  headerpos);
      headers.push_back("tcp_t");
      if (tcpptr->th_dport == htons(53)) {
        std::cerr << "We do not support DNS over TCP yet!" << endl;
        assert(false);
      }
    } else if (ip_protocol == 17) {
      struct udphdr *udpptr = (struct udphdr*)(pkt.data() +  headerpos);
      headers.push_back("udp_t");
      if (udpptr->uh_dport == htons(53)) {
        headers.push_back("dns_t");
      }
    }
  }
  return headers;
}
