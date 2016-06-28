#!/bin/bash
#
# testbed: Simulation environment for PFPSim Framework models
#
# Copyright (C) 2016 Concordia Univ., Montreal
#     Samar Abdi
#     Umair Aftab
#     Gordon Bailey
#     Faras Dewal
#     Shafigh Parsazad
#     Eric Tremblay
#
# Copyright (C) 2016 Ericsson
#     Bochra Boughzala
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#
################################################################################
#Bash script to execute the model over varying client-server loads
#Author: f_dewal@encs.concordia.ca
################################################################################
rm *.csv
rm *.pcap
rm timingReport.txt
rm -rf logs
rm -rf pcaps
rm LB_Logs

mkdir LB_Logs
mkdir LB_Logs/static
mkdir LB_Logs/shortest_queue
mkdir LB_Logs/round_robin

taskset -a -cp 3 $$

echo "{ \"policy\" : \"round_robin\" }" > ../Configs/LoadBalancer.cfg
{ time ../../build/testbed-sim -c ../Configs/ -Xp4 ../testbed_nat_router.json -Xtpop ../testbedRoutingTable_LB_test.txt -v profile > logger ; } 2>>timingReport.txt
mv -t ./LB_Logs/round_robin/ *.pcap *.csv logger timingReport.txt

echo "{ \"policy\" : \"static\" }" > ../Configs/LoadBalancer.cfg
{ time ../../build/testbed-sim -c ../Configs/ -Xp4 ../testbed_nat_router.json -Xtpop ../testbedRoutingTable_LB_test.txt -v profile > logger ; } 2>>timingReport.txt
mv -t ./LB_Logs/static/ *.pcap *.csv logger timingReport.txt

echo "{ \"policy\" : \"shortest_queue\" }" > ../Configs/LoadBalancer.cfg
{ time ../../build/testbed-sim -c ../Configs/ -Xp4 ../testbed_nat_router.json -Xtpop ../testbedRoutingTable_LB_test.txt -v profile > logger ; } 2>>timingReport.txt
mv -t ./LB_Logs/shortest_queue/ *.pcap *.csv logger timingReport.txt
