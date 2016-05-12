#!/bin/bash

pfpdb npu-sim --args "-Xp4 simple_router.json -Xtpop table.txt -v minimal -Xin Configs/input.pcap -Xvalidation-out output.pcap" -v 
