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
taskset -a -cp 2 $$
mkdir logs
for i in `seq 1 1 2000`; do
	if [ ! -d logs/log"$1" ]; then
		mkdir logs/log"$1"
	fi
	taskset 0x4 top -b -H -n 1 -o -PID -w 100 > logs/log"$1"/cpulogs_"$1"_"$i".txt
	sleep 1
done
