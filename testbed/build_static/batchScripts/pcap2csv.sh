#!/bin/bash

####################################################################################
# Bash script to convert pcap files to payload and request csvs
#
# Author: f_dewal@encs.concordia.ca
####################################################################################

DIR="pcaps/"
PTDIR="csvs_PayloadTransmitted/"
RDIR="csvs_Requests/"
csv=".csv"

if [ -d $PTDIR ]; then
	rm -rf $PTDIR
fi
if [ -d $RDIR ]; then
	rm -rf $RDIR
fi


mkdir $PTDIR
mkdir $RDIR

for entry in `ls $DIR`; do
	echo $entry
	mkdir $PTDIR$entry
	mkdir $RDIR$entry
	for filename in `ls $DIR$entry`; do
		fname=`echo $filename | cut -d'.' -f 1`
		#echo $DIR$entry/$filename

		tshark -r $DIR$entry/$filename -Y "tcp and tcp.len!=0"         -T fields -e frame.number -e frame.time -e ip.src -e ip.dst -e ip.proto -e col.Length -e tcp.len -e col.Info -E header=y -E separator=, -E quote=d -E occurrence=f > $PTDIR$entry/$fname$csv
		tshark -r $DIR$entry/$filename -Y "tcp and tcp.flags.reset==1" -T fields -e frame.number -e frame.time -e ip.src -e ip.dst -e ip.proto -e col.Length -e tcp.len -e col.Info -E header=y -E separator=, -E quote=d -E occurrence=f > $RDIR$entry/$fname$csv
		
	#ssconvert $CDIR$fname$csv $XDIR$fname$excel
	done
done
