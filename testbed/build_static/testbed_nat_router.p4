/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header_type ipv4_t {
    fields {
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr: 32;
    }
}

parser start {
    return parse_ethernet;
}

#define ETHERTYPE_IPV4 0x0800

header ethernet_t ethernet;

parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4;
        default: ingress;
    }
}

header ipv4_t ipv4;

field_list ipv4_checksum_list {
        ipv4.version;
        ipv4.ihl;
        ipv4.diffserv;
        ipv4.totalLen;
        ipv4.identification;
        ipv4.flags;
        ipv4.fragOffset;
        ipv4.ttl;
        ipv4.protocol;
        ipv4.srcAddr;
        ipv4.dstAddr;
}

field_list_calculation ipv4_checksum {
    input {
        ipv4_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field ipv4.hdrChecksum  {
    verify ipv4_checksum;
    update ipv4_checksum;
}

#define IP_PROT_TCP 0x06
#define IP_PROT_UDP 0x11

header_type meta_nat_t {
    fields {
        ipv4_sa : 32;
        ipv4_da : 32;
        srcp : 16;
        dstp : 16;
        tcpLength : 16;
    }
}

metadata meta_nat_t meta_nat;

parser parse_ipv4 {
    extract(ipv4);
    set_metadata(meta_nat.ipv4_sa, ipv4.srcAddr);
    set_metadata(meta_nat.ipv4_da, ipv4.dstAddr);
    set_metadata(meta_nat.tcpLength, ipv4.totalLen - 20);
    return select(ipv4.protocol) {
        IP_PROT_TCP : parse_tcp;
        IP_PROT_UDP : parse_udp;
        default : ingress;
    }
}

header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 4;
        flags : 8;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}

header tcp_t tcp;

parser parse_tcp {
    extract(tcp);
    set_metadata(meta_nat.srcp, tcp.srcPort);
    set_metadata(meta_nat.dstp, tcp.dstPort);
    return ingress;
}

field_list tcp_checksum_list {
        ipv4.srcAddr;
        ipv4.dstAddr;
        8'0;
        ipv4.protocol;
        meta_nat.tcpLength;
        tcp.srcPort;
        tcp.dstPort;
        tcp.seqNo;
        tcp.ackNo;
        tcp.dataOffset;
        tcp.res;
        tcp.flags;
        tcp.window;
        tcp.urgentPtr;
        payload;
}

field_list_calculation tcp_checksum {
    input {
        tcp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field tcp.checksum {
    verify tcp_checksum if(valid(tcp));
    update tcp_checksum if(valid(tcp));
}

header_type udp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        len : 16;
        checksum : 16;
    }
}

header udp_t udp;

parser parse_udp {
    extract(udp);
    set_metadata(meta_nat.srcp, udp.srcPort);
    set_metadata(meta_nat.dstp, udp.dstPort);
    return ingress;
}

field_list udp_checksum_list {
        ipv4.srcAddr;
        ipv4.dstAddr;
        8'0;
        ipv4.protocol;
        udp.len;
        udp.srcPort;
        udp.dstPort;
        payload;
}

field_list_calculation udp_checksum {
    input {
        udp_checksum_list;
    }
    algorithm : csum16;
    output_width : 16;
}

calculated_field udp.checksum {
    verify udp_checksum if(valid(udp));
    update udp_checksum if(valid(udp));
}

action _drop() {
    drop();
}

header_type routing_metadata_t {
    fields {
        nhop_ipv4 : 32;
    }
}

action perform_forward_nat(server_ipv4) {
    modify_field(ipv4.dstAddr, server_ipv4);
}

table forward_nat {
    reads {
        meta_nat.ipv4_sa : lpm;    // this is the client ID
        meta_nat.ipv4_da : exact;
    }
    actions {
        perform_forward_nat;
    }
    size: 1024;
}

action perform_reverse_nat(node_ipv4) {
  modify_field(ipv4.srcAddr, node_ipv4);
}

table reverse_nat {
  reads {
    meta_nat.ipv4_sa : lpm;   // this is the server instance ip
  }
  actions {
    perform_reverse_nat;
  }
}

metadata routing_metadata_t routing_metadata;

action set_nhop(nhop_ipv4, port) {
    modify_field(routing_metadata.nhop_ipv4, nhop_ipv4);
    modify_field(standard_metadata.egress_port, port);
    add_to_field(ipv4.ttl, -1);
}

table ipv4_lpm {
    reads {
        ipv4.dstAddr : lpm;
    }
    actions {
        set_nhop;
        _drop;
    }
    size: 1024;
}

action set_dmac(dmac) {
    modify_field(ethernet.dstAddr, dmac);
}

table forward {
    reads {
        routing_metadata.nhop_ipv4 : exact;
    }
    actions {
        set_dmac;
        _drop;
    }
    size: 512;
}

action rewrite_mac(smac) {
    modify_field(ethernet.srcAddr, smac);
}

table send_frame {
    reads {
        standard_metadata.egress_port: exact;
    }
    actions {
        rewrite_mac;
        _drop;
    }
    size: 256;
}

control ingress {
    if(valid(ipv4) and ipv4.ttl > 0) {
        apply(forward_nat);
        apply(reverse_nat);
        apply(ipv4_lpm);
        apply(forward);
    }
}

control egress {
    apply(send_frame);
}
