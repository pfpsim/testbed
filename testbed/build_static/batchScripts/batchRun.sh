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

#!/bin/bash
################################################################################
#Bash script to execute the model over varying client-server loads
#Author: f_dewal@encs.concordia.ca
################################################################################
rm *.csv
rm *.pcap
rm timingReport.txt
rm -rf logs
rm -rf pcaps

mkdir logs
mkdir pcaps

taskset -a -cp 3 $$

#edit config file to increase instances
for i in `seq 1 1 10`; do
  echo "******RUN****** : Number of clients: "$i >> timingReport.txt
  echo "{ \"virtualInstances\" : \""$i"\" }" > ../Configs/client_tcp_test.cfg
  ./loggingScript.sh $i &
  { time ../../build/testbed-sim -c ../Configs/ -Xp4 ../testbed_router.json -Xtpop ../testbedRoutingTable.txt -v minimal ; } 2>>timingReport.txt
  mkdir pcaps/pcaps"$i"
  mv ./ingress_sctime.pcap ./pcaps/pcaps"$i"/switch_in.pcap
  mv ./egress_sctime.pcap ./pcaps/pcaps"$i"/switch_out.pcap

  pkill -f loggingScript.sh
done
