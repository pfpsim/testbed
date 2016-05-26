#!/bin/bash
taskset -a -cp 2 $$
mkdir logs
for i in `seq 1 1 2000`; do
	if [ ! -d logs/log"$1" ]; then
		mkdir logs/log"$1"
	fi
	taskset 0x4 top -b -H -n 1 -o -PID -w 100 > logs/log"$1"/cpulogs_"$1"_"$i".txt
	sleep 1
done
