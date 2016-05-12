#include "TestbedUtilities.h"
#include <utility>
#include <string>
#include <vector>
#include <map>

ClientConfigStruct TestbedUtilities::getClientConfigurations
  (std::map<std::string, std::string> configMap, std::string configFile) {
  ClientConfigStruct ncs;
  try {
    std::vector<std::string> tempvec;
    std::string tempstr;

    ncs.node_type = configMap.find("type")->second;
    if (ncs.node_type.compare("client") != 0) {
      std::cerr << "Incorrect config file " << configFile
        << " used for client configuration!" << endl;
      assert(false);
    }

    tempstr = configMap.find("simulationTime")->second;

    tempvec = getStringVector(tempstr);

    if (tempvec.size() < 2) {
      std:cerr << configFile << ": Incorrect simTime specified! Usage."
      <<" \"12 SC_SEC\" "
      <<endl;
      assert(false);
    }

    if (tempvec.at(1).compare("SEC") == 0) {
      ncs.end_time = sc_time(stod(tempvec.at(0)), SC_SEC);
    } else if (tempvec.at(1).compare("MS") == 0) {
      ncs.end_time = sc_time(stod(tempvec.at(0)), SC_MS);
    } else if (tempvec.at(1).compare("US") == 0) {
      ncs.end_time = sc_time(stod(tempvec.at(0)), SC_US);
    } else if (tempvec.at(1).compare("NS") == 0) {
      ncs.end_time = sc_time(stod(tempvec.at(0)), SC_NS);
    } else if (tempvec.at(1).compare("PS") == 0) {
      ncs.end_time = sc_time(stod(tempvec.at(0)), SC_PS);
    } else if (tempvec.at(1).compare("FS") == 0) {
      ncs.end_time = sc_time(stod(tempvec.at(0)), SC_FS);
    } else {
      std::cerr << configFile << ": Server/ Client Unknown unit for delay "
        << "specified!"
        << " \nSupported values are: SC_SEC, SC_MS, SC_US, SC_NS, SC_PS, SC_FS"
        << endl;
      assert(false);
    }
    tempstr = configMap.find("archive")->second;

    if (tempstr.compare("true") == 0) {
      ncs.archive = true;
    } else {
      ncs.archive = false;
    }
    tempstr = configMap.find("headers")->second;

    ncs.list = getStringVector(tempstr);
    if (ncs.list.size() == 0) {
      std::cerr << "No headers specified!" << endl;
      assert(false);
    }
    tempstr = configMap.find("delays")->second;
    tempvec.clear();
    tempvec = getStringVector(tempstr);

    for (int index = 0; index < tempvec.size(); index++) {
      tempstr = configMap.find("delayUnit")->second;
      if (tempstr.compare("SEC") == 0) {
        ncs.delay.delay_values.push_back(
            sc_time(stod(tempvec.at(index)), SC_SEC));
      } else if (tempstr.compare("MS") == 0) {
        ncs.delay.delay_values.push_back(
            sc_time(stod(tempvec.at(index)), SC_MS));
      } else if (tempstr.compare("US") == 0) {
        ncs.delay.delay_values.push_back(
            sc_time(stod(tempvec.at(index)), SC_US));
      } else if (tempstr.compare("NS") == 0) {
        ncs.delay.delay_values.push_back(
            sc_time(stod(tempvec.at(index)), SC_NS));
      } else if (tempstr.compare("PS") == 0) {
        ncs.delay.delay_values.push_back(
            sc_time(stod(tempvec.at(index)), SC_PS));
      } else if (tempstr.compare("FS") == 0) {
        ncs.delay.delay_values.push_back(
            sc_time(stod(tempvec.at(index)), SC_FS));
      } else {
        std::cerr << "Server/ Client Unknown unit for delay specified! \n"
            << "Supported values are: SC_SEC, SC_MS, SC_US,"
            << "SC_NS, SC_PS, SC_FS" << endl;
        assert(false);
      }
    }
    tempvec.clear();
    tempstr = configMap.find("delayDist")->second;
    tempvec = getStringVector(tempstr);
    ncs.delay.distribution.type = tempvec.at(0);

    if (tempvec.size() < 2) {
      getDefaultDistributionParameters(ncs.delay.distribution.type,
          &ncs.delay.distribution.param1,
          &ncs.delay.distribution.param2);
    } else if (tempvec.size() < 3) {
      ncs.delay.distribution.param1 = stod(tempvec.at(1));
      ncs.delay.distribution.param2 = 0;
    } else {
      ncs.delay.distribution.param1 = stod(tempvec.at(1));
      ncs.delay.distribution.param2 = stod(tempvec.at(2));
    }
    tempvec.clear();

    int headerCount = stoi(configMap.find("virtualInstances")->second);
    std::vector<std::string> clientIPs, serverIPs;

    tempstr = configMap.find("cl_dnspolicy")->second;
    tempvec = getStringVector(tempstr);
    std::string cl_dnsdist = tempvec.at(0);
    double cl_dnsp1, cl_dnsp2;
    if (tempvec.size() < 2) {
      getDefaultDistributionParameters(cl_dnsdist, &cl_dnsp1, &cl_dnsp2);
    } else if (tempvec.size() < 3) {
      cl_dnsp1 = stod(tempvec.at(1));
      cl_dnsp2 = 0;
    } else {
      cl_dnsp1 = stod(tempvec.at(1));
      cl_dnsp2 = stod(tempvec.at(2));
    }
    tempvec.clear();

    std::map<int, int> cl_dnsfreq;
    tempstr = configMap.find("cl_dnsmsq")->second;
    tempvec = getStringVector(tempstr);
    int rnum = -1;
    for (int index = 0; index < headerCount; index++) {
      if (cl_dnsdist.compare("round-robin") == 0) {
        rnum++;
        if (rnum == tempvec.size()) {
          rnum = 0;
        }
      } else {
        rnum = getRandomNum(0, tempvec.size() - 1, cl_dnsdist,
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
      // mask index: it->first
      // mask freq : it->second
      std::vector<std::string> ips;
      ips = getIPv4List(tempvec.at(it->first), it->second);
      if (ips.size() < it->second) {
        std::cerr << configFile << ": Unable to create sufficient clientIPs to "
        << "satisfy the configured dns policy!" << endl;
        assert(false);
      }
      for (std::string tempip : ips) {
        clientIPs.push_back(tempip);
      }
      ips.clear();
    }
    tempvec.clear();

    tempstr = configMap.find("se_dnspolicy")->second;
    tempvec = getStringVector(tempstr);
    std::string se_dnsdist = tempvec.at(0);
    double se_dnsp1, se_dnsp2;
    if (tempvec.size() < 2) {
      getDefaultDistributionParameters(se_dnsdist, &se_dnsp1, &se_dnsp2);
    } else if (tempvec.size() < 3) {
      se_dnsp1 = stod(tempvec.at(1));
      se_dnsp2 = 0;
    } else {
      se_dnsp1 = stod(tempvec.at(1));
      se_dnsp2 = stod(tempvec.at(2));
    }
    tempvec.clear();

    std::map<int, int> se_dnsfreq;
    tempstr = configMap.find("se_dnsmsq")->second;
    tempvec = getStringVector(tempstr);
    rnum = -1;
    for (int index = 0; index < headerCount; index++) {
      if (se_dnsdist.compare("round-robin") == 0) {
        rnum++;
        if (rnum == tempvec.size()) {
          rnum = 0;
        }
      } else {
        rnum = getRandomNum(0, tempvec.size() - 1, se_dnsdist,
          se_dnsp1, se_dnsp2);
      }
      if (se_dnsfreq.find(rnum) == se_dnsfreq.end()) {
        se_dnsfreq.insert(std::pair<int, int>(rnum, 1));
      } else {
        se_dnsfreq[rnum]++;
      }
    }
    for (std::map<int, int>::iterator it = se_dnsfreq.begin();
      it != se_dnsfreq.end(); ++it) {
      // mask index: it->first
      // mask freq : it->second
      std::vector<std::string> ips;
      ips = getIPv4List(tempvec.at(it->first), it->second);
      if (ips.size() < it->second) {
        std::cerr << configFile
        << ": Unable to create sufficient serverIPs to satisfy"
        << "the configured dns policy!" << endl;
        assert(false);
      }
      for (std::string tempip : ips) {
        serverIPs.push_back(tempip);
      }
      ips.clear();
    }
    tempvec.clear();
    for (int index = 0; index < headerCount; index++) {
      std::vector<uint8_t> header;
      for (int nhdr = 0; nhdr < ncs.list.size(); nhdr++) {
        if (ncs.list[nhdr].compare("ethernet_t") == 0) {
          struct ether_header eth_f;
          if (ncs.list.size() >= 2 &&
              ncs.list[nhdr + 1].compare("ipv4_t") == 0) {
            eth_f.ether_type = ntohs(2048);
          } else if (ncs.list.size() >= 2 &&
              ncs.list[nhdr + 1].compare("ipv6_t") == 0) {
            eth_f.ether_type = 0x86DD;
          } else {
            eth_f.ether_type = ntohs(0);
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
          int tos = stoi(configMap.find("tos")->second);
          ip_f.ip_tos = (uint8_t) tos;
          uint16_t len = 16;
          ip_f.ip_len = htons(len);
          ip_f.ip_id = htons(
            static_cast<int16_t>(getRandomNum(0, SHRT_MAX, "uniform")));
          ip_f.ip_off = 0;
          int ttl = stoi(configMap.find("ttl")->second);
          ip_f.ip_ttl = (uint8_t) ttl;
          // ipv4_ttl = ip_f.ip_ttl;
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

          std::string ipsrc, ipdst;
          // serverIPs, clientIPs
          ipdst = serverIPs.at(index);
          ipsrc = clientIPs.at(index);

          if (inet_aton(ipdst.c_str(), &ip_f.ip_dst) == 0) {
            std::cerr << "We got an invalid IP address! Dst: " << ipdst
                << endl;
            assert(false);
          }
          if (inet_aton(ipsrc.c_str(), &ip_f.ip_src) == 0) {
            std::cerr << "We got an invalid IP address! Src: " << ipsrc
                << endl;
            assert(false);
          }

          uint8_t *data =  static_cast<uint8_t*>(static_cast<void*>(&ip_f));
          header.insert(header.end(), data, data + sizeof(struct ip));
        } else if (ncs.list[nhdr].compare("tcp_t") == 0) {
          struct tcphdr tcp_f;
          std::string strport = configMap.find("sport")->second;
          uint16_t port;
          if (strport.compare("random") == 0) {
            port = getRandomNum(10000, 50000, "uniform");
          } else {
            port = (uint16_t) stoi(strport);
          }
          tcp_f.th_sport = htons(port);
          strport = configMap.find("dport")->second;
          if (strport.compare("random") == 0) {
            port = getRandomNum(10000, 50000, "uniform");
          } else {
            port = (uint16_t) stoi(strport);
          }
          tcp_f.th_dport = htons(port);
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
          std::string strport = configMap.find("sport")->second;
          uint16_t port;
          if (strport.compare("random") == 0) {
            port = getRandomNum(10000, 50000, "uniform");
          } else {
            port = (uint16_t) stoi(strport);
          }
          udp_f.uh_sport = htons(port);
          strport = configMap.find("dport")->second;
          if (strport.compare("random") == 0) {
            port = getRandomNum(10000, 50000, "uniform");
          } else {
            port = (uint16_t) stoi(strport);
          }
          udp_f.uh_dport = htons(port);
          udp_f.uh_ulen = 0; /* udp length */
          udp_f.uh_sum = 0; /* udp checksum */
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
    std::cerr << "Exception while parsing client/server nodes!" << endl;
    assert(false);
  }
  return ncs;
}
ServerConfigStruct TestbedUtilities::getServerConfigurations
  (std::map<std::string, std::string> configMap, std::string configFile) {
  ServerConfigStruct ncs;
  try {
    std::vector<std::string> tempvec;
    std::string tempstr;
    ncs.node_type = configMap.find("type")->second;
    if (ncs.node_type.compare("server") != 0) {
      std::cerr << "Incorrect config file " << configFile
        << " used for server configuration!" << endl;
      assert(false);
    }
    tempstr = configMap.find("archive")->second;
    if (tempstr.compare("true") == 0) {
      ncs.archive = true;
    } else {
      ncs.archive = false;
    }
    tempstr = configMap.find("headers")->second;
    ncs.list = getStringVector(tempstr);
    if (ncs.list.size() == 0) {
      std::cerr << "No headers specified!" << endl;
      assert(false);
    }
    tempvec.clear();
    tempstr = configMap.find("sizes")->second;
    tempvec = getStringVector(tempstr);
    tempstr = configMap.find("sizeUnit")->second;
    for (int index = 0; index < tempvec.size(); index++) {
      int32_t tempSize = stol(tempvec.at(index));
      if (tempstr.compare("kB") == 0) {
        tempSize *= 1024;
      } else if (tempstr.compare("MB") == 0) {
        tempSize *= 1024;
        tempSize *= 1024;
      } else if (tempstr.compare("GB") == 0) {
        tempSize *= 1024;
        tempSize *= 1024;
        tempSize *= 1024;
      }
      ncs.fsize.size_values.push_back(tempSize);
    }
    tempvec.clear();

    tempstr = configMap.find("mtu")->second;
    tempvec = getStringVector(tempstr);
    ncs.mtu = stoi(tempvec.at(0));
    tempstr = tempvec.at(1);
    if (tempstr.compare("kB") == 0) {
      ncs.mtu *= 1024;
    } else if (tempstr.compare("MB") == 0) {
      ncs.mtu *= 1024;
      ncs.mtu *= 1024;
    } else if (tempstr.compare("GB") == 0) {
      ncs.mtu *= 1024;
      ncs.mtu *= 1024;
      ncs.mtu *= 1024;
    }
    tempvec.clear();

    if (configMap.find("datarate") != configMap.end()) {
      tempstr = configMap.find("datarate")->second;
      tempvec = getStringVector(tempstr);
      ncs.datarate = stol(tempvec.at(0));
      tempstr = tempvec.at(1);
      if (tempstr.compare("kbps") == 0) {
        ncs.datarate *= 1024;
      } else if (tempstr.compare("Mbps") == 0) {
        ncs.datarate *= 1024;
        ncs.datarate *= 1024;
      } else if (tempstr.compare("Gbps") == 0) {
        ncs.datarate *= 1024;
        ncs.datarate *= 1024;
        ncs.datarate *= 1024;
      }
      tempvec.clear();
    }
    tempstr = configMap.find("sizeDist")->second;
    tempvec = getStringVector(tempstr);
    ncs.fsize.distribution.type = tempvec.at(0);
    if (tempvec.size() < 2) {
      getDefaultDistributionParameters(ncs.fsize.distribution.type,
          &ncs.fsize.distribution.param1,
          &ncs.fsize.distribution.param2);
    } else if (tempvec.size() < 3) {
      ncs.fsize.distribution.param1 = stod(tempvec.at(1));
      ncs.fsize.distribution.param2 = 0;
    } else {
      ncs.fsize.distribution.param1 = stod(tempvec.at(1));
      ncs.fsize.distribution.param2 = stod(tempvec.at(2));
    }
    tempvec.clear();
  } catch (std::exception &e) {
    std::cerr << "Exception while parsing server nodes! Config file: "
      << configFile << endl;
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

uint8_t TestbedUtilities::stringTouint16(const std::string& input) {
  std::string lut = "0123456789ABCDEF";

  char fc = toupper(input[0]);
  char sc = toupper(input[1]);

  int fn = lut.find(fc);
  int sn = lut.find(sc);

  uint8_t output = fn << 4 | sn;

  return output;
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
      /*
      struct ip6_hdr {
        uint32_t ip6_un1_flow; // 4 bits version, 8 bits TC, 20 bits flow-ID
        uint16_t ip6_un1_plen; // payload length
        uint8_t ip6_un1_nxt; // next header
        uint8_t ip6_un1_hlim; // hop limit
        // uint8_t ip6_un2_vfc; // 4 bits version, top 4 bits tclass
        struct in6_addr ip6_src; // source address
        struct in6_addr ip6_dst; // destination address
      };
      */
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
      tcpptr->th_sum = calculateTCPChecksum(pkt, tcpPos, 06, ipptr);
    } else if (hdr.compare("udp_t") == 0) {
      // extract header to be updated
      int udpPos = headerPos.back();
      headerPos.pop_back();
      int ipPos = headerPos.back();
      udpptr = (struct udphdr*) (dataptr + udpPos);
      ipptr = (struct ip*) (dataptr + ipPos);
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
  //  Assuming Ethernet, IP and TCP/ UDP headers
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


  if (headers.at(2).compare("tcp_t") == 0) {
    reqTCP = (struct tcphdr*) (reqPacket->setData().data() +
      sizeof(struct ether_header) + sizeof(struct tcphdr));
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
  } else if (headers.at(2).compare("udp_t") == 0) {
    reqUDP = (struct udphdr*) (reqPacket->setData().data() +
      sizeof(struct ether_header) + sizeof(struct tcphdr));
    resUDP.uh_sport = reqUDP->uh_sport;
    resUDP.uh_dport = reqUDP->uh_dport;
    resUDP.uh_ulen = 0;
    resUDP.uh_sum = 0;
    data =  static_cast<uint8_t*>(static_cast<void*>(&resUDP));
    resPacket->setData().insert(resPacket->setData().end(),
      data,  data + sizeof(struct udphdr));
  }
  // struct trackhdr resTrack;
}

std::vector<std::string> TestbedUtilities::getIPv4List(std::string dnsmsq,
  int maxListSize) {
  std::vector<std::string> ipv4List;
  std::stringstream dnsmsqss(dnsmsq);
  std::string mask, prefix;
  std::getline(dnsmsqss, mask, '/');
  std::getline(dnsmsqss, prefix, '/');
  std::bitset<32> andMask, orMask;
  std::bitset<32> maskbits, minIP, maxIP;
  int bitsav = 32-stoi(prefix);

  if (bitsav == 0) {
    ipv4List.push_back(mask);
  } else {
    andMask = std::bitset<32>().set();
    for (int index = 0; index < bitsav; index++) {
      andMask[index].flip();
    }
    orMask = ~andMask;
    struct in_addr temp;
    inet_aton(mask.c_str(), &temp);
    maskbits = std::bitset<32>(ntohl(temp.s_addr));

    minIP = std::bitset<32>(maskbits & andMask);
    uint32_t minIPlong = minIP.to_ulong();

    maxIP = std::bitset<32>(maskbits | orMask);
    uint32_t maxIPlong = maxIP.to_ulong();
    minIPlong = minIPlong+5;
    while (minIPlong <= maxIPlong) {
      temp.s_addr = htonl(minIPlong);
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
  // int totalVectorSize = 0;

  for (int tvindex = 0; tvindex < cps2.size(); tvindex++) {
    // totalVectorSize += cps2.at(tvindex).size();
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

std::string TestbedUtilities::getConnectionID(
  const std::vector<uint8_t> &packet, const std::vector<std::string> &list,
  const std::string &type) {
  struct ip *ipptr;
  std::string id;
  // size_t header_pos = 0;
  for (std::string header_name : list) {
    if (header_name.compare("ipv4_t")) {
      ipptr = (struct ip*)(packet.data() + sizeof(ether_header));
      if (type.compare("src") == 0) {
        id = inet_ntoa(ipptr->ip_src);
      } else {
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
