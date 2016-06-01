#!/bin/bash
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
####################################################################################
# Bash script to convert pcap files to payload and request csvs
#
# Author: f_dewal@encs.concordia.ca
####################################################################################

DIR="pcaps/"
PTDIR="csvs_PayloadTransmitted/"
RDIR="csvs_Requests/"
TDDIR="csvs_TotalData/"
csv=".csv"

if [ -d $PTDIR ]; then
	rm -rf $PTDIR
fi
if [ -d $RDIR ]; then
	rm -rf $RDIR
fi
if [ -d $TDDIR ]; then
	rm -rf $TDDIR
fi


mkdir $PTDIR
mkdir $RDIR
mkdir $TDDIR

for entry in `ls $DIR`; do
	echo $entry
	mkdir $PTDIR$entry
	mkdir $RDIR$entry
	mkdir $TDDIR$entry
	for filename in `ls $DIR$entry`; do
		fname=`echo $filename | cut -d'.' -f 1`
		#echo $DIR$entry/$filename

		tshark -r $DIR$entry/$filename -Y "tcp and tcp.len!=0"         -T fields -e frame.number -e frame.time -e ip.src -e ip.dst -e ip.proto -e col.Length -e tcp.len -e col.Info -E header=y -E separator=, -E quote=d -E occurrence=f > $PTDIR$entry/$fname$csv
		tshark -r $DIR$entry/$filename -Y "tcp and tcp.flags.reset==1" -T fields -e frame.number -e frame.time -e ip.src -e ip.dst -e ip.proto -e col.Length -e tcp.len -e col.Info -E header=y -E separator=, -E quote=d -E occurrence=f > $RDIR$entry/$fname$csv
		tshark -r $DIR$entry/$filename -T fields -e frame.number -e frame.time -e ip.src -e ip.dst -e ip.proto -e col.Length -e tcp.len -e col.Info -E header=y -E separator=, -E quote=d -E occurrence=f > $TDDIR$entry/$fname$csv

	#ssconvert $CDIR$fname$csv $XDIR$fname$excel
	done
done
