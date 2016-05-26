import time
import os
import math
from datetime import datetime
import os, os.path
print("\n"*10)

summaryD=open('systemLog.csv','w')
summaryD.write("Configuration,avg_load_1,avg_load_5,avg_load_15,th_total,th_running,th_sleeping,cpu%_us,cpu%_sy,cpu%_ni,cpu%_id,cpu%_wa,")
summaryD.write("cpu%_hi,cpu%_si,mem_total_KiB,mem_usage_KiB\n")

directoriesList=[name for name in os.listdir('.') if not os.path.isfile(name)]
print(directoriesList)
#for DIR in directoriesList:
for dindex in range(1, 400):
 DIR="logs/log"+str(dindex)+"/"
 if(os.path.isdir(DIR)):
  print(DIR)
  #debugf=open("logSummary/log_"+str(dindex)+".csv",'w')
  #debugf.write("Configuration,avg_load_1,avg_load_5,avg_load_15,th_total,th_running,th_sleeping,cpu%_us,cpu%_sy,cpu%_ni,cpu%_id,cpu%_wa,")
  #debugf.write("cpu%_hi,cpu%_si,mem_total_KiB,mem_usage_KiB\n")
  config=dindex*2
  load_1=[]
  load_5=[]
  load_15=[]
  th_total=[]
  th_running=[]
  th_sleeping=[]
  cpu_us=[]
  cpu_sy=[]
  cpu_ni=[]
  cpu_id=[]
  cpu_wa=[]
  cpu_hi=[]
  cpu_si=[]
  mem_total=[]
  mem_use=[]
  max_load_1=0
  max_load_5=0
  max_load_15=0
  max_th_total=0
  max_th_running=0
  max_th_sleeping=0
  max_cpu_us=0
  max_cpu_sy=0
  max_cpu_ni=0
  max_cpu_id=0
  max_cpu_wa=0
  max_cpu_hi=0
  max_cpu_si=0
  max_mem_total=0
  max_mem_use=0
  for findex in range(1,400):
   th_total_count=0
   th_running_count=0
   th_sleeping_count=0
   file=DIR+"cpulogs_"+str(dindex)+"_"+str(findex)+".txt"
   if(os.path.isfile(file)):
    fhandle=open(file,'r')
    line=0
    while(1):
     line=line+1
     fline=fhandle.readline()
     fline.strip()
     if(fline=='\n'):
      continue
     if(fline==''):
      break
     if(line == 1):
      elements=fline.split(',')
      temp=float(elements[2].split(':')[1].strip())
      load_1.append(temp)
      if(temp > max_load_1):
       max_load_1=temp
      temp=float(elements[3].strip())
      load_5.append(temp)
      if(temp > max_load_5):
       max_load_5=temp
      temp=float(elements[4].strip())
      load_15.append(temp)
      if(temp > max_load_15):
       max_load_15=temp
     if(line == 6):
      elements=fline.split(',')
      
      temp=float(elements[0].split(':')[1].strip().split()[0].strip())
      cpu_us.append(temp)
      if(temp > max_cpu_us):
       max_cpu_us = temp 
      
      temp=float(elements[1].strip().split()[0])
      cpu_sy.append(temp)
      if(temp > max_cpu_sy):
       max_cpu_sy = temp 
      
      temp=float(elements[2].strip().split()[0])
      cpu_ni.append(temp)
      if(temp > max_cpu_ni):
       max_cpu_ni = temp 
      
      temp=float(elements[3].strip().split()[0])
      cpu_id.append(temp)
      if(temp > max_cpu_id):
       max_cpu_id = temp 
      
      temp=float(elements[4].strip().split()[0])
      cpu_wa.append(temp)
      if(temp > max_cpu_wa):
       max_cpu_wa = temp 
      
      temp=float(elements[5].strip().split()[0])
      cpu_hi.append(temp)
      if(temp > max_cpu_hi):
       max_cpu_hi = temp 
      
      temp=float(elements[6].strip().split()[0])
      cpu_si.append(temp)
      if(temp > max_cpu_si):
       max_cpu_si = temp 

     if(line == 7):
      elements=fline.split(',')
      temp=float(elements[0].split(':')[1].strip().split()[0].strip())
      mem_total.append(temp)
      if(temp > max_mem_total):
       max_mem_total=temp
      temp=float(elements[1].strip().split()[0])
      mem_use.append(temp)
      if(temp > max_mem_use):
       max_mem_use=temp
	  
  
     if(line > 10):
      elements=fline.split()
      if(elements[14]=="3"):
       th_total_count=th_total_count + 1
       if(elements[7]=="S" or elements[7]=="D"):
        th_sleeping_count=th_sleeping_count + 1
       if(elements[7]=="R"):
        th_running_count=th_running_count+1
    th_total.append(th_total_count)
    if(th_total_count > max_th_total):
     max_th_total = th_total_count
    th_running.append(th_running_count)
    if(th_running_count > max_th_running):
      max_th_running = th_running_count
    th_sleeping.append(th_sleeping_count)
    if(th_sleeping_count > max_th_sleeping):
      max_th_sleeping = th_sleeping_count
  #for lindex in range(0, len(load_1)):
   #debugf.write("%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"%(config,float(load_1[lindex]),float(load_5[lindex]),float(load_15[lindex]),float(th_total[lindex]),float(th_running[lindex]),float(th_sleeping[lindex]),float(cpu_us[lindex]),float(cpu_sy[lindex]),float(cpu_ni[lindex]),float(cpu_id[lindex]),float(cpu_wa[lindex]),float(cpu_hi[lindex]),float(cpu_si[lindex]),float(mem_total[lindex]),float(mem_use[lindex])))
  summaryD.write("%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n"% (config, max_load_1, max_load_5, max_load_15, max_th_total, max_th_running, max_th_sleeping, max_cpu_us, max_cpu_sy, max_cpu_ni, max_cpu_id, max_cpu_wa, max_cpu_hi, max_cpu_si,max_mem_total, max_mem_use))
  #summaryD.write("%d,%f,%f,%f,%f,"%(dindex, dstddev_iat, cstddev_iat, dstddev_size, cstddev_size))
summaryD.close()
