#! /bin/bash

# no. of processors set here
np=`grep processor /proc/cpuinfo|wc -l`
rm -f results* resultIdx* analyse*.log

i=0
while [ $i -lt $np ]; do
  etierra analyseGenebank.tcl $np $i &>analyse$i.log&
  rr="$rr results$i"
  ri="$rr resultIdx$i"
  i=$[$i+1]
done
 
wait

BDBmerge $rr results
BDBmerge $ri resultIdx

