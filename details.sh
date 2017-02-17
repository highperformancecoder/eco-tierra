#! /bin/bash

# no. of processors set here
np=`grep processor /proc/cpuinfo|wc -l`
rm -f results* details*.log

i=0
while [ $i -lt $np ]; do
  details.tcl $i $np >details$i.log&
  rr="$rr results$i"
  i=$[$i+1]
done
 
wait

BDBmerge $rr results

etierra rem-neutrals.tcl
